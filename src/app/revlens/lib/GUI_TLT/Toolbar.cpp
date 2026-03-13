
#include "RlpRevlens/GUI_TLT/Toolbar.h"

#include "RlpCore/PY/Interp.h"

#include "RlpGui/WIDGET/Slider.h"

#include "RlpRevlens/CNTRL/Video.h"
#include "RlpRevlens/CNTRL/Audio.h"



RLP_SETUP_LOGGER(revlens,GUI_TLT, Toolbar)

#ifdef SCAFFOLD_IOS
const int GUI_TLT_Toolbar::TB_HEIGHT = 40;
#else
const int GUI_TLT_Toolbar::TB_HEIGHT = 35;
#endif


GUI_TLT_Toolbar::GUI_TLT_Toolbar(GUI_ItemBase* parent):
    GUI_ItemBase(parent),
    _currTrackUuidStr("")
{
    setHeight(TB_HEIGHT);
    // _layout = new GUI_HLayout(this);
    // _layout->setSpacing(5);

    int btnXPos = 5;
    #ifdef SCAFFOLD_IOS
    int btnXInc = 25;
    #else
    int btnXInc = 20;
    #endif


    _prevFrameButton = new GUI_IconButton(
        ":PioFiveMusicWhite/saltar-a-pista-anterior.png",
        this
    );
    _prevFrameButton->setPos(btnXPos, 5);

    connect(
        _prevFrameButton,
        &GUI_IconButton::buttonPressed,
        this,
        &GUI_TLT_Toolbar::onPrevFrame
    );


    _playButton = new GUI_ToggleIconButton(this,
        ":PioFiveMusicWhite/music-player-play.png",
        ":PioFiveMusicWhite/music-player-pause-lines.png");


    btnXPos += btnXInc;

    _playButton->setPos(btnXPos, 6);

    _nextFrameButton = new GUI_IconButton(
        ":PioFiveMusicWhite/skip-track-option.png",
        this
    );

    btnXPos += btnXInc;

    _nextFrameButton->setPos(btnXPos, 5);
    connect(
        _nextFrameButton,
        &GUI_IconButton::buttonPressed,
        this,
        &GUI_TLT_Toolbar::onNextFrame
    );

    int currFrameLabelXPos = 75;
    int currFrameLabelYPos = 7;

    #ifdef SCAFFOLD_IOS
    currFrameLabelXPos += 5;
    currFrameLabelYPos += 3;
    #endif

    _currFrameLabel = new GUI_Label(this, "-");
    _currFrameLabel->setWidth(60);
    _currFrameLabel->setPos(currFrameLabelXPos, currFrameLabelYPos);
    _currFrameLabel->setTextColor(QColor(120, 120, 200));

    QFont f = _currFrameLabel->font();
    // f.setPixelSize(GUI_Style::FontPixelSizeDefault);
    f.setBold(true);

    _currFrameLabel->setFont(f);

    _startFrameLabel = new GUI_Label(this, "-");
    _startFrameLabel->setWidth(30);
    _startFrameLabel->setPos(125, currFrameLabelYPos);

    f = _startFrameLabel->font();
    f.setPixelSize(GUI_Style::FontPixelSizeDefault - 3);
    _startFrameLabel->setFont(f);

    _endFrameLabel = new GUI_Label(this, "-");
    _endFrameLabel->setWidth(30);
    _endFrameLabel->setFont(f);
    _endFrameLabel->setPos(parent->width() - 75, currFrameLabelYPos);

    // _toolButton = new GUI_SvgButton(":feather/menu.svg", this, 20, 4);
    // _toolButton->setPos(parent->width() - 30, 8);
    _volButton = new GUI_ToggleSvgButton(this, ":misc/mute.svg", ":misc/volume.svg", 18);
    _volButton->setToolTipText("Mute / Unmute, Volume Control");
    _volButton->setPos(parent->width() - 30, 8);

    GUI_Slider* volSlider = new GUI_Slider(
        _volButton,
        GUI_Slider::O_SL_VERTICAL
    );
    _volButton->menu()->addItem(volSlider);
    _volButton->menu()->setWidth(volSlider->width());

    connect(
        _volButton,
        &GUI_ToggleSvgButton::onButtonToggled,
        this,
        &GUI_TLT_Toolbar::onMuteToggled
    );

    connect(
        volSlider,
        &GUI_Slider::moving,
        this,
        &GUI_TLT_Toolbar::onVolumeChangeRequested
    );

    CNTRL_MainController* cntrl = CNTRL_MainController::instance();
    _tltView = new GUI_TLT_View(cntrl, this);
    _tltView->setPos(160, 3);

    cntrl->getVideoManager()->registerProbe(this);

    initConnections();

    _tltView->setWidth(parent->width() - timelineWidthOffset());

    DS_SessionPtr session = cntrl->session();
    onInFrameChanged(session->inFrame());
    onOutFrameChanged(session->outFrame());
}


GUI_TLT_Toolbar::~GUI_TLT_Toolbar()
{
    RLP_LOG_DEBUG("")
    CNTRL_MainController::instance()->getVideoManager()->deregisterProbe(this);
}

void
GUI_TLT_Toolbar::initConnections()
{
    CNTRL_MainController* cntrl = CNTRL_MainController::instance();

    connect(
        cntrl,
        &CNTRL_MainController::playStateChanged,
        this,
        &GUI_TLT_Toolbar::onPlayStateChanged
    );

    connect(
        cntrl->session().get(),
        &DS_Session::inFrameChanged,
        this,
        &GUI_TLT_Toolbar::onInFrameChanged
    );

    connect(
        cntrl->session().get(),
        &DS_Session::outFrameChanged,
        this,
        &GUI_TLT_Toolbar::onOutFrameChanged
    );

    connect(
        cntrl->session().get(),
        &DS_Session::frameCountChanged,
        this,
        &GUI_TLT_Toolbar::onFrameCountChanged
    );

    connect(
        cntrl,
        &CNTRL_MainController::noticeEvent,
        this,
        &GUI_TLT_Toolbar::onNoticeEvent
    );

    connect(
        _playButton,
        &GUI_ToggleIconButton::buttonPressed,
        this,
        &GUI_TLT_Toolbar::onPlayPausePressed
    );


    // connect(
    //     _toolButton,
    //     &GUI_SvgButton::buttonPressed,
    //     this,
    //     &GUI_TLT_Toolbar::onToolsToggled
    // );
}


int
GUI_TLT_Toolbar::timelineWidthOffset()
{
    return 240;
    /*
    int widgetOffset = _playButton->width() +
                       _currFrameLabel->width() +
                       _startFrameLabel->width() +
                       _endFrameLabel->width();

    return _playButton->width() + widgetOffset + 60;
    */
}


void
GUI_TLT_Toolbar::syncUpdate(qlonglong currFrame)
{
    _tltView->syncUpdate(currFrame);

    _currFrameLabel->setText(QString("%1").arg(currFrame));
    _currFrameLabel->setWidth(60);
    _currFrameLabel->update();


    // RLP_LOG_DEBUG(currFrame)
}


void
GUI_TLT_Toolbar::syncUpdateImmediate(qlonglong frameNum)
{
    syncUpdate(frameNum);
}


void
GUI_TLT_Toolbar::onPlayPausePressed(QVariantMap md)
{
    RLP_LOG_INFO("")

    CNTRL_MainController* cntrl = qobject_cast<CNTRL_MainController*>(
        CNTRL_MainController::instance()
    );

    cntrl->runCommand("TogglePlayState");

    _playButton->update();
}



void
GUI_TLT_Toolbar::onPrevFrame(QVariantMap /* metadata */)
{
    // RLP_LOG_DEBUG("")

    CNTRL_MainController* cntrl = CNTRL_MainController::instance();
    cntrl->gotoFrame(cntrl->currentFrameNum() - 1);

}


void
GUI_TLT_Toolbar::onNextFrame(QVariantMap /* metadata */)
{
    RLP_LOG_DEBUG("")

    CNTRL_MainController* cntrl = CNTRL_MainController::instance();
    cntrl->gotoFrame(cntrl->currentFrameNum() + 1);

}


void
GUI_TLT_Toolbar::onMuteToggled(QVariantMap md)
{
    // RLP_LOG_DEBUG(md)
    CNTRL_Audio* audioCntrl = CNTRL_MainController::instance()->getAudioManager();

    bool isToggled = md.value("toggled").toBool();
    int button = md.value("button").toInt();

    if (button == 2)
    {
        bool vis = !_volButton->menu()->isVisible();
        QPointF mpos = _volButton->mapToItem(
            dynamic_cast<QQuickItem*>(_scene), QPointF(15, 15));

        _volButton->menu()->setPos(mpos);
        _volButton->menu()->setVisible(vis);

        return;
    } else if (button == -1)
    {
        // from API call
        _volButton->toggle();

    } else
    {
        // hide menu if its open on left-click
        _volButton->menu()->setVisible(false);
    }

    QString msg;

    if (isToggled)
    {
        msg = "Mute";
        audioCntrl->mute();
    } else
    {
        msg = "Unmute";
        audioCntrl->unmute();
    }

    QVariantList args;
    args.append(msg);
    PY_Interp::call("revlens.cmds.display_message", args);
}


void
GUI_TLT_Toolbar::onVolumeChangeRequested(float val)
{
    if (val < 0.001) // to help computation of logarithm close to 0
    {
        val = 0.001;
    }

    qreal volVal = QtAudio::convertVolume(
        val, QtAudio::LogarithmicVolumeScale, QtAudio::LinearVolumeScale);

    // RLP_LOG_DEBUG(val << volVal)

    CNTRL_Audio* audioCntrl = CNTRL_MainController::instance()->getAudioManager();
    audioCntrl->setVolume(volVal);

    QString msg = QString("Set Volume: %1").arg(val);

    QVariantList args;
    args.append(msg);
    PY_Interp::call("revlens.cmds.display_message", args);
}


void
GUI_TLT_Toolbar::onInFrameChanged(qlonglong frame)
{
    // RLP_LOG_DEBUG(frame)

    _startFrameLabel->setText(QString("%1").arg(frame));
    _startFrameLabel->update();

    _tltView->setInFrame(frame);
}


void
GUI_TLT_Toolbar::onFrameCountChanged(qlonglong frameCount)
{
    // RLP_LOG_DEBUG(frameCount)

    onInFrameChanged(1);
    onOutFrameChanged(frameCount);
}


void
GUI_TLT_Toolbar::onOutFrameChanged(qlonglong frame)
{
    // RLP_LOG_DEBUG(frame)

    _endFrameLabel->setText(QString("%1").arg(frame));
    _endFrameLabel->update();

    _tltView->setOutFrame(frame);
}


void
GUI_TLT_Toolbar::onPlayStateChanged(int playState)
{
    DS_ControllerBase::PlaybackState state = (DS_ControllerBase::PlaybackState)playState;

    if (state == DS_ControllerBase::PLAYSTATE_PLAYING)
    {
        _playButton->setToggled(true);
    } else
    {
        _playButton->setToggled(false);
    }

    _playButton->update();
}


void
GUI_TLT_Toolbar::onNoticeEvent(QString evtName, QVariantMap evtInfo)
{
    // RLP_LOG_DEBUG(evtName <<  evtInfo);

    if (evtName == "playlist.changed")
    {
        QString trackUuid = evtInfo.value("track_uuid").toString();

        _currTrackUuidStr = trackUuid;

        DS_TrackPtr track = _controller->session()->getTrackByUuid(trackUuid);
        if (track != nullptr)
        {
            qlonglong trackMaxFrameNum = track->getMaxFrameNum();

            // RLP_LOG_DEBUG("Got track:" << trackUuid << "max frame num:" << trackMaxFrameNum)

            onInFrameChanged(1);
            onOutFrameChanged(trackMaxFrameNum);

        } else
        {
            RLP_LOG_ERROR("Invalid track Uuid")
        }

    } else if (evtName == "audio.request_toggle_mute")
    {
        onMuteToggled(evtInfo);
    }
}


void
GUI_TLT_Toolbar::onParentSizeChanged(qreal width, qreal height)
{
    // RLP_LOG_DEBUG(width << height)

    setWidth(width);
    setHeight(TB_HEIGHT);

    _endFrameLabel->setPos(width - 75, 6);
    // _toolButton->setPos(width - 30, 8);
    _volButton->setPos(width - 30, 8);

    _tltView->updateWidth(width - timelineWidthOffset());
}


void
GUI_TLT_Toolbar::paint(GUI_Painter* painter)
{
    QColor bgColor = GUI_Style::PaneBg;
    painter->setPen(bgColor);
    painter->setBrush(bgColor);
    painter->drawRect(boundingRect());
}