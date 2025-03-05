//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include "RlpRevlens/CNTRL/Video.h"
#include "RlpRevlens/DISP/GLView.h"

#include "RlpRevlens/GUI_TL2/MediaTrack.h"
#include "RlpRevlens/GUI_TL2/MediaItem.h"
#include "RlpRevlens/GUI_TL2/TrackManager.h"
#include "RlpRevlens/GUI_TL2/View.h"
#include "RlpRevlens/GUI_TL2/TrackToggleButtonImage.h"

RLP_SETUP_LOGGER(revlens, GUI_TL2, MediaTrack)

GUI_TL2_MediaTrack::GUI_TL2_MediaTrack(GUI_TL2_TrackManager* mgr, DS_TrackPtr strack, QString trackName):
    GUI_TL2_Track::GUI_TL2_Track(mgr, strack, trackName),
    _heightText(20),
    _heightAudio(20),
    _heightVideo(40),
    _visText(true),
    _visThumbnail(true),
    _visWaveform(true)
{
    initTrackConnections();
    initGearMenu();
    initButtons();

    #ifndef SCAFFOLD_WASM
    setButtonsVisible(false, /*optionalOnly=*/true);
    #endif

    setHeight(trackHeight());
    onParentSizeChanged(view()->width(), view()->height());
}


void
GUI_TL2_MediaTrack::initCurrSession()
{
    RLP_LOG_DEBUG("")

    QVariantList nodeList = _strack->getNodeFrameList();
    for (int ni=0; ni != nodeList.size(); ++ni)
    {
        QVariantMap nfInfo = nodeList.at(ni).toMap();
        qlonglong frameNum = nfInfo.value("frame").toLongLong();
        QString nodeUuidStr = nfInfo.value("node_uuid").toString();

        DS_NodePtr node = _strack->getNodeByFrame(frameNum);

        qlonglong mediaFrameCount = node->getProperty<qlonglong>("media.frame_count");

        if (node == nullptr)
        {
            RLP_LOG_ERROR("Invalid node returned at frame" << frameNum)
            continue;
        }

        addMediaItem(node, frameNum, mediaFrameCount);
    }

}


void
GUI_TL2_MediaTrack::initTrackConnections()
{
    RLP_LOG_DEBUG("");
    
    connect(
        _strack.get(),
        SIGNAL(trackCleared()),
        this,
        SLOT(clearItems())
    );

    connect(
        _strack.get(),
        SIGNAL(mediaRemoved(DS_NodePtr, QString, qlonglong)),
        this,
        SLOT(onMediaRemoved(DS_NodePtr, QString, qlonglong))
    );
}


void
GUI_TL2_MediaTrack::initButtons()
{
    GUI_SvgToggleButton* visButton = new GUI_SvgToggleButton(
        ":feather/lightgrey/eye.svg",
        ":feather/lightgrey/eye-off.svg",
        this, 18, 0, GUI_Style::TrackHeaderFg
    );
    visButton->setToolTipText("Toggle Visibility");

    connect(
        visButton,
        &GUI_SvgButton::buttonPressed,
        this,
        &GUI_TL2_MediaTrack::onVisibilityChanged
    );

    
    addButton("visibility", visButton);

    GUI_SvgButton* gearButton = new GUI_SvgButton(
        ":feather/lightgrey/settings.svg", this, 18, 0, GUI_Style::TrackHeaderFg
    );

    gearButton->setToolTipText("Gear Menu - click for available actions");
    connect(
        gearButton,
        &GUI_SvgButton::buttonPressed,
        this,
        &GUI_TL2_MediaTrack::onGearMenuClicked
    );

    addButton("gear_menu", gearButton);

    // GUI_TL2_TrackToggleButtonImage* thumbButton = new GUI_TL2_TrackToggleButtonImage(
    //     this,
    //     ":/picture_gray.png"
    // );

    GUI_SvgButton* thumbButton = new GUI_SvgButton(
        ":feather/lightgrey/image.svg", this, 18, 0, GUI_Style::TrackHeaderFg
    );
    thumbButton->setOutlined(_visThumbnail);
    thumbButton->setToggled(_visThumbnail);
    thumbButton->setToolTipText("Toggle Thumbnail");
    thumbButton->icon()->setOutlinedBgColour(GUI_Style::TrackHeaderIconBg);

    connect(
        thumbButton,
        &GUI_SvgButton::buttonPressed,
        this,
        &GUI_TL2_MediaTrack::onThumbnailClicked
    );

    addButton("thumbnail", thumbButton);

    GUI_TL2_TrackToggleButtonImage* waveformButton = new GUI_TL2_TrackToggleButtonImage(
        this,
        ":/audio-wave.png"
    );
    waveformButton->setToolTipText("Toggle Waveform");
    waveformButton->setToggled(_visWaveform);
    waveformButton->setBgColOn(QColor(60, 60, 60));
    connect(
        waveformButton,
        SIGNAL(toggleChanged(bool)),
        this,
        SLOT(setWaveformVisible(bool))
    );

    addButton("waveform", waveformButton);


    GUI_TL2_TrackToggleButtonImage* prevItemButton = new GUI_TL2_TrackToggleButtonImage(
        this,
        ":/PioFiveMusicWhite/saltar-a-pista-anterior_trackgray.png"
    );
    prevItemButton->setToolTipText("Jump to previous media");
    connect(
        prevItemButton,
        SIGNAL(toggleChanged(bool)),
        this,
        SLOT(onPrevItemClicked(bool))
    );
    addButton("prev_item", prevItemButton);

    GUI_TL2_TrackToggleButtonImage* nextItemButton = new GUI_TL2_TrackToggleButtonImage(
        this,
        ":/PioFiveMusicWhite/skip-track-option_trackgray.png"
    );
    nextItemButton->setToolTipText("Jump to next media");
    connect(
        nextItemButton,
        SIGNAL(toggleChanged(bool)),
        this,
        SLOT(onNextItemClicked(bool))
    );
    addButton("next_item", nextItemButton);

    /*
    self.btn_end = IconButton(
            'PioFiveMusicWhite/fast-forward-arrow.png',
            tooltip='Previous Frame'
        )

        self.btn_playpause = ToggleButton(
            'PioFiveMusicWhite/music-player-play.png',
            'PioFiveMusicWhite/music-player-pause-lines.png',
            tooltip='Play / Pause'
        )

        self.btn_next_frame = IconButton(
            'PioFiveMusicWhite/skip-track-option.png',
            tooltip='Next Frame'
        )
    */

}


GUI_TL2_MediaTrack::~GUI_TL2_MediaTrack()
{
    // RLP_LOG_DEBUG("")
}


qreal
GUI_TL2_MediaTrack::trackHeight()
{
    int height = 20; //  _heightDefault;
    if (_visText)
    {
        height += _heightText;
    }

    if (_visThumbnail) {
        height += _heightVideo;
    }

    if (_visWaveform) {
        height += _heightAudio;
    }

    // RLP_LOG_DEBUG(height)

    return height;
}


int
GUI_TL2_MediaTrack::heightVideo()
{
    int height = 0; // _heightDefault;
    if (_visThumbnail) {
        height = _heightVideo;
    }

    return height;
}


int
GUI_TL2_MediaTrack::heightText()
{
    int result = 30;
    if (_visThumbnail)
    {
        result += _heightVideo;
    }

    if (_visWaveform)
    {
        result += _heightAudio;
    }

    return result;
}


void
GUI_TL2_MediaTrack::onParentSizeChanged(qreal nwidth, qreal nheight)
{
    GUI_TL2_Track::onParentSizeChanged(nwidth, nheight);
    // setWidth(nwidth);
}


void
GUI_TL2_MediaTrack::addMediaItem(DS_NodePtr node, qlonglong startFrame, qlonglong mediaFrameCount)
{
    RLP_LOG_DEBUG("startFrame:" << startFrame << "mediaFrameCount:" << mediaFrameCount);


    if (_itemMap.contains(node->graph()->uuid()))
    {

        RLP_LOG_DEBUG("Item already in track, updating");
        GUI_TL2_MediaItem* item = dynamic_cast<GUI_TL2_MediaItem*>(_itemMap.value(node->graph()->uuid()));

        _itemFrameMap.remove(item->startFrame());
        _itemFrameMap.insert(startFrame, item->uuid());

        item->setStartFrame(startFrame);
        item->setFrameCount(mediaFrameCount);

        connect(
            item,
            &GUI_TL2_MediaItem::itemSelected,
            this,
            &GUI_TL2_MediaTrack::onItemSelected
        );


    } else
    {
        GUI_TL2_MediaItem* item = new GUI_TL2_MediaItem(this, node, startFrame, mediaFrameCount);

        connect(
            item,
            &GUI_TL2_MediaItem::itemSelected,
            this,
            &GUI_TL2_MediaTrack::onItemSelected
        );

        addItem(item);
    }

}


void
GUI_TL2_MediaTrack::onVisibilityChanged(QVariantMap md)
{
    // RLP_LOG_DEBUG(md)

    GUI_ButtonBase* visButton = getButtonByName("visibility");
    bool vis = !visButton->isToggled();

    visButton->setToggled(vis);
    visButton->setOutlined(vis);

    _strack->setEnabled(!vis);
}


void
GUI_TL2_MediaTrack::onMediaRemoved(DS_NodePtr node, QString trackUuidStr, qlonglong startFrame)
{
    RLP_LOG_DEBUG("");

    clearItem(node->graph()->uuid().toString());
}


void
GUI_TL2_MediaTrack::setTextVisible(bool vis)
{
    RLP_LOG_VERBOSE("")

    _visText = vis;
    setHeight(trackHeight());
    _trackManager->refresh();
}


void
GUI_TL2_MediaTrack::onThumbnailClicked(QVariantMap md)
{
    GUI_ButtonBase* thumb = getButtonByName("thumbnail");

    RLP_LOG_DEBUG(md << thumb->isToggled())

    bool vis = !thumb->isToggled();
    setThumbnailVisible(vis);
}


void
GUI_TL2_MediaTrack::setThumbnailVisible(bool vis)
{
    GUI_ButtonBase* thumb = getButtonByName("thumbnail");

    thumb->setToggled(vis);
    thumb->setOutlined(vis);

    _visThumbnail = vis;

    setHeight(trackHeight());
    _trackManager->refresh();
}

void
GUI_TL2_MediaTrack::setWaveformVisible(bool vis)
{
    _visWaveform = vis;
    setHeight(trackHeight());
    _trackManager->refresh();
}


void
GUI_TL2_MediaTrack::onPrevItemClicked(bool vis)
{
    RLP_LOG_DEBUG("");

    CNTRL_MainController* cntrl = dynamic_cast<CNTRL_MainController*>(CNTRL_MainController::instance());

    qlonglong currFrame = cntrl->currentFrameNum();
    DS_NodePtr node = _strack->getNodeByFrame(currFrame);
    if (node == nullptr) {
        RLP_LOG_ERROR("No node at current frame for this track");
        return;
    }


    QString nUuidStr = node->graph()->uuid().toString();

    QVariantList nodeFrameList = _strack->getNodeFrameList();

    // get the index of the node at the current frame
    QVariantList::iterator it;
    int idx = 0;
    bool found = false;
    RLP_LOG_DEBUG("iteratoring");

    for (it=nodeFrameList.begin(); it != nodeFrameList.end(); ++it) {
        
        QString iNodeUuidStr = it->toMap().value("node_uuid").toString();
        RLP_LOG_DEBUG(iNodeUuidStr);
        if (iNodeUuidStr == nUuidStr) {
            RLP_LOG_DEBUG("found node at index " << idx);
            found = true;
            break;
        }

        idx++;
    }

    if (found && (idx >= 1)) {
        QVariant nextInfo = nodeFrameList.at(idx - 1);
        QString nextNodeUuid = nextInfo.toMap().value("node_uuid").toString();
        qlonglong nextNodeFrame = nextInfo.toMap().value("frame").toLongLong();

        RLP_LOG_DEBUG("Next node at " << nextNodeFrame << " with uuid: " << nextNodeUuid);

        cntrl->gotoFrame(nextNodeFrame, true, true);
    }
}


void
GUI_TL2_MediaTrack::onNextItemClicked(bool vis)
{
    RLP_LOG_DEBUG("");

    CNTRL_MainController* cntrl = dynamic_cast<CNTRL_MainController*>(CNTRL_MainController::instance());

    qlonglong currFrame = cntrl->currentFrameNum();
    DS_NodePtr node = _strack->getNodeByFrame(currFrame);
    if (node == nullptr) {
        RLP_LOG_ERROR("No node at current frame for this track");
        return;
    }

    QString nUuidStr = node->graph()->uuid().toString();

    QVariantList nodeFrameList = _strack->getNodeFrameList();

    // get the index of the node at the current frame
    QVariantList::iterator it;
    int idx = 0;
    bool found = false;
    for (it=nodeFrameList.begin(); it != nodeFrameList.end(); ++it) {
        
        QString iNodeUuidStr = it->toMap().value("node_uuid").toString();
        RLP_LOG_DEBUG(iNodeUuidStr);
        if (iNodeUuidStr == nUuidStr) {
            RLP_LOG_DEBUG("found node at index " << idx);
            found = true;
            break;
        }

        idx++;
    }

    if ((found) && (idx < (nodeFrameList.size() - 1))) {

        QVariant nextInfo = nodeFrameList.at(idx + 1);
        QString nextNodeUuid = nextInfo.toMap().value("node_uuid").toString();
        qlonglong nextNodeFrame = nextInfo.toMap().value("frame").toLongLong();

        RLP_LOG_DEBUG("Next node at " << nextNodeFrame << " with uuid: " << nextNodeUuid);

        cntrl->gotoFrame(nextNodeFrame, true, true);
    }
}


void
GUI_TL2_MediaTrack::onItemSelected(QVariantMap info)
{
    RLP_LOG_DEBUG("")

    info.insert("track_uuid", uuid().toString());

    RLP_LOG_DEBUG(info)

    emit itemSelected(info);
}

/*
void
GUI_TL2_MediaTrack::paint(GUI_Painter* painter)
{
    GUI_TL2_Track::paint(painter, option, widget);

    
}
*/