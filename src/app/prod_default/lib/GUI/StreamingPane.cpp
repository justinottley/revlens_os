

#include "RlpProd/GUI/StreamingPane.h"

#include "RlpProd/CNTRL/StreamingController.h"

#include "RlpRevlens/CNTRL/MainController.h"
#include "RlpRevlens/CNTRL/Server.h"

#include "RlpCore/UTIL/Convert.h"
#include "RlpCore/UTIL/AppGlobals.h"

RLP_SETUP_LOGGER(prod, GUI, StreamListCallback)
RLP_SETUP_LOGGER(prod, GUI, StreamingPane)

GUI_StreamListCallback::GUI_StreamListCallback(GUI_StreamingPane* pane):
    _pane(pane)
{
}


void
GUI_StreamListCallback::onResultReady(QVariantMap result)
{
    // RLP_LOG_DEBUG(result)

    QString replyStr = result.value("result").toMap().value("result").toString();
    QVariantMap reply = UTIL_Convert::fromJsonString(replyStr);

    _pane->initStreamList(reply);
}


GUI_StreamingPane::GUI_StreamingPane(GUI_ItemBase* parent):
    GUI_ItemBase(parent),
    _drawOverlay(true),
    _msg("Nothing loaded")
{
    _toolbar = new GUI_ToolGroup(this, GUI_ToolGroup::TG_HORIZONTAL, GUI_ToolGroup::TG_ALIGN_NONE, 25);
    _toolbar->layout()->setSpacing(10);
    _toolbar->setZ(10); // workaround for frame overlapping this slightly....
    _toolbar->addSpacer(5);

    _currentStream = new GUI_Label(this, "-");
    _toolbar->addItem(_currentStream, 4);
    _toolbar->addDivider(4);

    _refreshButton = new GUI_SvgButton(
        ":feather/refresh-cw.svg",
        this
    );
    connect(
        _refreshButton,
        &GUI_SvgButton::buttonPressed,
        this,
        &GUI_StreamingPane::onRefreshRequested
    );

    _snapshotButton = new GUI_SvgButton(":feather/camera.svg", _toolbar, 20);
    connect(
        _snapshotButton,
        &GUI_SvgButton::buttonPressed,
        this,
        &GUI_StreamingPane::onSnapshotRequested
    );

    _clearAnnotationsButton = new GUI_SvgButton(":misc/clean.svg", _toolbar, 20);
    connect(
        _clearAnnotationsButton,
        &GUI_SvgButton::buttonPressed,
        this,
        &GUI_StreamingPane::onClearAnnotationsRequested
    );

    _toolbar->addItem(_refreshButton, 5);
    _toolbar->addItem(_snapshotButton, 5);
    _toolbar->addItem(_clearAnnotationsButton, 5);
    _toolbar->addDivider(4);

    _playButton = new GUI_ToggleIconButton(
        _toolbar,
        ":feather/play-circle.svg", //":/PioFiveMusicWhite/music-player-play.png",
        ":feather/stop-circle.svg", // ":/PioFiveMusicWhite/music-player-pause-lines.png",
        20
    );

    QColor onCol(200, 200, 200);
    QColor offCol(200, 40, 40);

    _playButton->svgIcon()->setFgColour(GUI_Style::IconBg);
    _playButton->svgIcon()->setBgColour(GUI_Style::IconBg);
    _playButton->offSvgIcon()->setFgColour(offCol);
    _playButton->offSvgIcon()->setBgColour(offCol);
    _playButton->setToolTipText("Play / Pause Stream");
    
    RLP_LOG_DEBUG("playbutton height:" << _playButton->height())

    connect(
        _playButton,
        &GUI_ToggleIconButton::buttonPressed,
        this,
        &GUI_StreamingPane::onPlayPausePressed
    );


    _streamList = new GUI_IconButton("", this, 20, 4);
    _streamList->setText("Pick a Stream:");
    _streamList->setOutlined(true);
    _streamList->setSvgIconPath(":feather/chevron-down.svg", 20);
    connect(
        qobject_cast<GUI_MenuPane*>(_streamList->menu()),
        &GUI_MenuPane::menuItemSelected,
        this,
        &GUI_StreamingPane::onStreamChanged
    );

    _toolbar->addItem(_streamList);
    _toolbar->addItem(_playButton, 3);
    
    _view = new DISP_GLView(this);
    _view->setVisible(false);
    _view->setPos(0, 15); // WTF!??!? // _toolbar->height());

    CNTRL_StreamingController* scntrl = CNTRL_StreamingController::instance();
    scntrl->getVideoManager()->registerDisplay(_view);
    _view->setVideoSourceIdx(scntrl->getVideoManager()->idx());

    _view->registerEventPlugin(scntrl->drawController()->duplicate());

    connect(
        scntrl,
        &CNTRL_StreamingController::playStateChanged,
        this,
        &GUI_StreamingPane::onPlayStateChanged
    );


    setPaintBackground(false);

    onRefreshRequested();

    onParentSizeChanged(parent->width(), parent->height());
}


void
GUI_StreamingPane::initStreamList(QVariantMap streamInfo)
{
    RLP_LOG_DEBUG("")
    QVariantList streams = streamInfo.value("items").toList();

    GUI_MenuPane* streamListMenu = qobject_cast<GUI_MenuPane*>(_streamList->menu());
    streamListMenu->clear();

    for (int i=0; i != streams.size(); ++i)
    {
        QVariantMap mtxStreamInfo = streams.at(i).toMap();

        // fixup serializataion
        QString encodedNameO = mtxStreamInfo.value("name").toString();
        QString encodedName = encodedNameO;
        while (encodedName.at(encodedName.size() - 1) == '.')
        {
            encodedName[encodedName.size() - 1] = '=';
        }

        QString streamInfoJson = QString(QByteArray::fromBase64(encodedName.toLatin1()));
        QVariantMap streamInfo = UTIL_Convert::fromJsonString(streamInfoJson);
        streamInfo.insert("encoded_name", encodedNameO);

        RLP_LOG_DEBUG(streamInfo)

        QString streamName = QString("%1 - %2 : %3").
            arg(streamInfo.value("user").toString()).
            arg(streamInfo.value("host").toString()).
            arg(streamInfo.value("screen").toString());

        GUI_MenuItem* item = streamListMenu->addItem(streamName, streamInfo);
    }
}


void
GUI_StreamingPane::onRefreshRequested(QVariantMap md)
{
    RLP_LOG_DEBUG("")

    CoreNet_RPCFuture* callback = new GUI_StreamListCallback(this);
    CNTRL_MainController::instance()->getServerManager()->siteClientSendCall(
        "stream.list", QVariantList(), QVariantMap(), QVariantMap(), callback);
}


void
GUI_StreamingPane::onStreamChanged(QVariantMap md)
{
    // RLP_LOG_DEBUG(md)

    QString text = md.value("text").toString();
    QVariantMap data = md.value("data").toMap();

    _streamList->setText(text);
    _streamInfo = data;

}


void
GUI_StreamingPane::onClearAnnotationsRequested(QVariantMap md)
{
    RLP_LOG_DEBUG(md)

    CNTRL_StreamingController* scntrl = CNTRL_StreamingController::instance();
    scntrl->clearAnnotations();
}


/*
void
GUI_StreamingPane::onGrabImageReady(QImage image)
{
    RLP_LOG_DEBUG(image.width() << image.height())

    QTemporaryFile f(QString("%1/stream_snapshot_XXXXXX.jpg").arg(QDir::tempPath()));
    f.setAutoRemove(false);
    f.open();
    QString fname = f.fileName();

    RLP_LOG_DEBUG(fname)

    image.save(f.fileName());

    RLP_LOG_DEBUG("Wrote" << f.fileName())

    // QImage* img = new QImage(image);
    QVariantMap md;
    md.insert("image_path", f.fileName());

    CNTRL_MainController::instance()->emitNoticeEvent("stream.snapshot_ready", md);
}
*/


void
GUI_StreamingPane::loadStream()
{
    QString streamName = _streamInfo.value("encoded_name").toString();
    if (streamName == "")
    {
        RLP_LOG_ERROR("invalid stream")
        return;
    }


    _msg = "Loading stream...";
    _drawOverlay = true;
    _view->setVisible(false);
    update();

    RLP_LOG_DEBUG(_streamInfo)

    QVariantMap* globals = UTIL_AppGlobals::instance()->globalsPtr();
    QString siteHostname = globals->value("edb.site_hostname").toString();
    // QString siteHostname = "postwebstage";
    QString streamUrl = QString("rtsp://%1:8554/%2").arg(siteHostname).arg(streamName);

    RLP_LOG_DEBUG(streamUrl)
    _currentStream->setText(_streamList->text());

    RLP_LOG_DEBUG(streamUrl)

    CNTRL_StreamingController* scntrl = CNTRL_StreamingController::instance();
    scntrl->loadStream(streamUrl, _streamInfo);
}


void
GUI_StreamingPane::onPlayPausePressed(QVariantMap md)
{
    RLP_LOG_DEBUG(md << _playButton->isToggled())

    CNTRL_StreamingController* scntrl = CNTRL_StreamingController::instance();

    if (_playButton->isToggled())
    {
        loadStream();

        // if (scntrl->getPlaybackState() == DS_ControllerBase::PLAYSTATE_UNKNOWN)
        // {
        //     loadStream();
        // } else
        // {
        //     scntrl->playStream();
        // }
    } else
    {
        if (_drawOverlay)
        {
            _msg = "Stream Stopped";
            update();
        }

        scntrl->stopStream();
    }
    // loadStream();
}


void
GUI_StreamingPane::onSnapshotRequested(QVariantMap md)
{
    RLP_LOG_DEBUG(md)
    DS_FrameBufferPtr fbuf = _view->frameBuffer();
    QImage i = fbuf->toQImage();
    // QTemporaryFile f(QString("%1/stream_snapshot_XXXXXX.jpg").arg(QDir::tempPath()));
    // f.setAutoRemove(false);
    // f.open();
    // QString fname = f.fileName();

    // RLP_LOG_DEBUG(fname)

    // i.save(fname);

    QVariantMap nmd;
    nmd.insert("image", i);
    // nmd.insert("image_path", fname);

    CNTRL_MainController::instance()->emitNoticeEvent("view.snapshot_ready", nmd);
}


void
GUI_StreamingPane::onPlayStateChanged(int state)
{
    RLP_LOG_DEBUG(state)

    if ((DS_ControllerBase::PlaybackState)state == DS_ControllerBase::PLAYSTATE_PLAYING)
    {
        _drawOverlay = false;
        _view->setVisible(true);
    } else if ((DS_ControllerBase::PlaybackState)state == DS_ControllerBase::PLAYSTATE_PAUSED)
    {
        _msg = "Stream Stopped";
        _drawOverlay = true;
        _view->setVisible(false);
    }

    update();
}


void
GUI_StreamingPane::onParentVisibilityChanged(QString name, bool isVisible)
{
    // RLP_LOG_DEBUG(name << isVisible)

    QString tabName;
    if (_metadata.contains("tab.name"))
    {
        tabName = _metadata.value("tab.name").toString();
        if ((tabName == name) && (isVisible))
        {
            qobject_cast<GUI_ItemBase*>(parentItem())->setPaintBackground(false);
        }
    }
}


void
GUI_StreamingPane::onParentSizeChanged(qreal width, qreal height)
{
    if (width == 0) return;
    if (height == 0) return;

    GUI_ItemBase* parent = qobject_cast<GUI_ItemBase*>(parentItem());
    qreal parentHeightOffset = parent->heightOffset();

    setWidth(width);
    setHeight(height);

    // RLP_LOG_DEBUG(parentHeightOffset << _toolbar->height())

    qreal vheight = height - parentHeightOffset - _toolbar->height();
    _view->setWidth(width);
    _view->setHeight(vheight);
    _view->onParentSizeChanged(width, vheight);
}


void
GUI_StreamingPane::paint(GUI_Painter* painter)
{

    if (!_drawOverlay)
    {
        return;
    }
    
    painter->setPen(GUI_Style::BgMidGrey);
    painter->setBrush(QBrush(GUI_Style::BgMidGrey));

    painter->drawRect(0, _toolbar->height(), width(), height() - _toolbar->height()); // whalf, y, whalf, trackRectHeight - 2);

    // RLP_LOG_DEBUG(0 << _toolbar->height() << width() << height() - _toolbar->height())

    painter->setPen(Qt::white);

    QFont f = painter->font();
    f.setBold(true);
    f.setPixelSize(30);
    painter->setFont(f);

    painter->drawText(
        width() / 2 - 120,
        height() / 2,
        _msg
    );

}