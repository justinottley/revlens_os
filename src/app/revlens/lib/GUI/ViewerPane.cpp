

#include "RlpGui/BASE/ToolGroup.h"
#include "RlpGui/BASE/Scene.h"
#include "RlpGui/WIDGET/Slider.h"

#include "RlpCore/PY/Interp.h"


#include "RlpRevlens/CNTRL/MainController.h"
#include "RlpRevlens/CNTRL/Video.h"
#include "RlpRevlens/CNTRL/Audio.h"

#include "RlpRevlens/GUI/ViewerPane.h"

RLP_SETUP_LOGGER(revlens, GUI, ViewerPane)


GUI_ViewerPane::GUI_ViewerPane(GUI_ItemBase* parent):
    GUI_ItemBase(parent),
    _showToolbar(true),
    _vthidden(false),
    _playh(false)
{
    setFlag(QQuickItem::ItemAcceptsDrops);

    // RLP_LOG_DEBUG(parent->heightOffset())


    _toolbar = new GUI_ToolGroup(
        this, GUI_ToolGroup::TG_HORIZONTAL, GUI_ToolGroup::TG_ALIGN_CENTER, TOOLBAR_HEIGHT);

    #ifdef SCAFFOLD_WASM
    _toolbar->layout()->setPos(0, 3);
    #else
    _toolbar->layout()->setPos(0, 1);
    #endif

    _view = new DISP_GLView(this);
    
    CNTRL_MainController* cntrl = CNTRL_MainController::instance();
    CNTRL_Video* vmgr = cntrl->getVideoManager();
    vmgr->registerDisplay(_view);

    CNTRL_Audio* amgr = cntrl->getAudioManager();
    connect(
        amgr,
        &CNTRL_Audio::volumeChanged,
        this,
        &GUI_ViewerPane::onReceiveVolumeChange
    );

    connect(
        _view,
        &DISP_GLView::displayDestroyed,
        vmgr,
        &CNTRL_Video::deregisterDisplay
    );

    #ifndef SCAFFOLD_IOS

    connect(
        _view,
        &DISP_GLView::hoverEnter,
        this,
        &GUI_ViewerPane::onViewHoverEnter
    );

    connect(
        _view,
        &DISP_GLView::hoverLeave,
        this,
        &GUI_ViewerPane::onViewHoverLeave
    );

    #endif

    connect(
        _view,
        &DISP_GLView::hoverMove,
        this,
        &GUI_ViewerPane::onViewHoverMove
    );

    connect(
        _view->contextMenu(),
        &GUI_MenuPane::menuItemSelected,
        this,
        &GUI_ViewerPane::onViewContextMenuItemSelected
    );

    connect(
        cntrl,
        &CNTRL_MainController::noticeEvent,
        this,
        &GUI_ViewerPane::onNoticeEvent
    );

    setPaintBackground(false);

    _loadOverlay = new GUI_ViewerLoadOverlay(_view);
    _loadOverlay->setVisible(false);

    _viewerTimeline = new GUI_ViewerTimeline();
    _view->registerEventPlugin(_viewerTimeline);
    _view->setPluginIndex(_viewerTimeline->name(), 0);

    // _vttimer.setInterval(2000);
    _vttimer.setSingleShot(true);
    _vttimer.callOnTimeout(this, &GUI_ViewerPane::checkHideOverlayTimeline);
    // _vtimeline->setVisible(false);

    initToolbar();

    onParentSizeChanged(parent->width(), parent->height());
}


GUI_ViewerPane::~GUI_ViewerPane()
{
    delete _toolbar;
    delete _view;
}


QQuickItem*
GUI_ViewerPane::create(GUI_ItemBase* parent, QVariantMap toolInfo)
{
    GUI_ViewerPane* vp = new GUI_ViewerPane(parent);
    
    QVariant v;
    v.setValue(vp->view());

    vp->setMetadata("view", v);

    CNTRL_MainController::instance()->emitToolCreated(QString("Viewer"), vp);

    return vp;
}


void
GUI_ViewerPane::initToolbar()
{
    RLP_LOG_DEBUG("")

    // return;


    // QString rootPath = "/home/justinottley/dev/revlens_root/sp_revlens/inst/Linux-x86_64/revlens/global";
    // QString iconPath = rootPath;
    // iconPath += "/static/icons/PioFiveMusicWhite/";
    QString iconPath = ":/PioFiveMusicWhite/";

    // -----

    QString path_start = iconPath;
    path_start += "rewind-arrow.png";
    
    GUI_IconButton* btnStart = new GUI_IconButton(path_start, _toolbar);
    btnStart->setToolTipText("Goto Start");
    _toolbar->addItem(btnStart);
    
    connect(
        btnStart,
        &GUI_IconButton::buttonPressed,
        this,
        &GUI_ViewerPane::onStartFrame
    );

    QString path_prev_frame = iconPath;
    path_prev_frame += "saltar-a-pista-anterior.png";

    GUI_IconButton* btnPrevFrame = new GUI_IconButton(path_prev_frame, _toolbar);
    btnPrevFrame->setToolTipText("Goto Previous Frame");
    _toolbar->addItem(btnPrevFrame);
    
    connect(
        btnPrevFrame,
        &GUI_IconButton::buttonPressed,
        this,
        &GUI_ViewerPane::onPrevFrame
    );

    QString path_play = iconPath;
    path_play += "music-player-play.png";

    QString path_pause = iconPath;
    path_pause += "music-player-pause-lines.png";

    _playButton = new GUI_ToggleIconButton(
        _toolbar,
        path_play,
        path_pause);

    _playButton->setToolTipText("Play / Pause");
    _toolbar->addItem(_playButton, 2);
    
    connect(
        _playButton,
        &GUI_IconButton::buttonPressed,
        this,
        &GUI_ViewerPane::onPlayPausePressed
    );

    QString path_next_frame = iconPath;
    path_next_frame += "skip-track-option.png";

    GUI_IconButton* btnNextFrame = new GUI_IconButton(
        path_next_frame,
        _toolbar
    );

    btnNextFrame->setToolTipText("Goto Next Frame");
    _toolbar->addItem(btnNextFrame);

    connect(
        btnNextFrame,
        &GUI_IconButton::buttonPressed,
        this,
        &GUI_ViewerPane::onNextFrame
    );

    QString path_end = iconPath;
    path_end += "fast-forward-arrow.png";
    GUI_IconButton* btnEnd = new GUI_IconButton(
        path_end,
        _toolbar
    );
    btnEnd->setToolTipText("Goto End");
    _toolbar->addItem(btnEnd);

    connect(
        btnEnd,
        &GUI_IconButton::buttonPressed,
        this,
        &GUI_ViewerPane::onEndFrame
    );

    _toolbar->addDivider(20, TOOLBAR_HEIGHT - 10);

    _vtbutton = new GUI_SvgButton(":misc/move.svg", this, DEFAULT_ICON_SIZE + 10);
    _vtbutton->icon()->setBgColour(QColor(180, 180, 180));
    // _vtbutton->setIconPos(0, -2);
    _vtbutton->setMetadata("toggled", false);
    _vtbutton->setToolTipText("Toggle Viewer Overlay Timeline");
    connect(
        _vtbutton,
        &GUI_SvgButton::buttonPressed,
        this,
        &GUI_ViewerPane::toggleOverlayTimeline
    );

    _toolbar->addItem(_vtbutton);

    _toolbar->addDivider(20, TOOLBAR_HEIGHT - 10);

    _volButton = new GUI_ToggleSvgButton(
        this, ":misc/mute.svg", ":misc/volume.svg"
    );
    _volButton->setToolTipText("Volume Control");

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
        &GUI_ViewerPane::onMuteToggled
    );

    connect(
        volSlider,
        &GUI_Slider::endMove,
        this,
        &GUI_ViewerPane::onVolumeChangeRequested
    );

    _toolbar->addItem(_volButton, 6);

    _toolbar->addDivider(20, TOOLBAR_HEIGHT - 10);

    // _viewerToolsButton = new GUI_SvgButton(":misc/compare.svg", this, DEFAULT_ICON_SIZE + 10);
    // _viewerToolsButton->icon()->setBgColour(QColor(180, 180, 180));
    // // _viewerToolsButton->setMetadata("toggled", true);
    // _viewerToolsButton->setToolTipText("Toggle Viewer Tools");

    // connect(
    //     _viewerToolsButton,
    //     &GUI_SvgButton::buttonPressed,
    //     this,
    //     &GUI_ViewerPane::onViewerToolsToggled
    // );

    // onCompToolsToggled(QVariantMap());

    // _toolbar->addItem(_viewerToolsButton, 1);


    _playModeButton = new GUI_PlayModeButton(this, DEFAULT_ICON_SIZE + 10);
    _playModeButton->setToolTipText("Play Mode");


    _toolbar->addSpacer(2);
    _toolbar->addItem(_playModeButton, 0);

    //int xCenter = (boundingRect().width() / 2) - (_toolbar->itemWidth() / 2);
    //_toolbar->setPos(xCenter, 0);
}


void
GUI_ViewerPane::onViewHoverEnter()
{
    // RLP_LOG_DEBUG("")

    // _view->setCursor(Qt::CrossCursor);

    if (_vtbutton->isToggled())
    {
        // RLP_LOG_DEBUG("")
        // _view->setSoloPluginByName("ViewerTimeline");
        DS_EventPlugin* vt = _view->getEventPluginByName("ViewerTimeline");
        if (vt != nullptr)
        {
            vt->setEnabled(true);
        }

        _view->update();
        _vthidden = false;
    }
}


void
GUI_ViewerPane::onViewHoverLeave()
{
    // RLP_LOG_DEBUG("")

    DS_EventPlugin* vt = _view->getEventPluginByName("ViewerTimeline");
    if (vt != nullptr)
    {
        vt->setEnabled(false);
    }
    // _view->resetSoloPlugin("ViewerTimeline");
    _view->update();
}


void
GUI_ViewerPane::onViewHoverMove()
{
    // RLP_LOG_DEBUG("")

    QPointF cpos = QCursor::pos();

    if ((cpos != _mpos) && (_vthidden))
    {
        onViewHoverEnter();
    }

    if (CNTRL_MainController::instance()->isPaused())
    {
        _vttimer.stop();
        _vttimer.setSingleShot(true);
        _vttimer.start(2000);

    } else if (!_playh)
    {
        _playh = true;
        _vttimer.start(2000);
    }
    
    _mpos = QCursor::pos();
}


void
GUI_ViewerPane::onViewContextMenuItemSelected(QVariantMap cmiInfo)
{
    RLP_LOG_DEBUG(cmiInfo)

    QString method = cmiInfo.value("data").toMap().value("callback").toString();
    PY_Interp::call(method, QVariantList());
}


void
GUI_ViewerPane::toggleOverlayTimeline(QVariantMap md)
{
    RLP_LOG_DEBUG(_vtbutton->isToggled())

    if (_vtbutton->isToggled())
    {
        _vtbutton->icon()->setOutlined(false);
        _vtbutton->setToggled(false);

        #ifdef SCAFFOLD_IOS
        onViewHoverLeave();
        #endif

    } else
    {
        RLP_LOG_DEBUG("turning on toggled")
        _vtbutton->icon()->setOutlined(true);
        _vtbutton->setToggled(true);

        #ifdef SCAFFOLD_IOS
        onViewHoverEnter();
        #endif

    }

    _vtbutton->icon()->update();
}


void
GUI_ViewerPane::checkHideOverlayTimeline()
{
    #ifdef SCAFFOLD_IOS
    return;
    #endif

    QPointF cpos = QCursor::pos();

    if (cpos == _mpos)
    {
        _view->resetSoloPlugin("ViewerTimeline");
        _view->update();

        if (_view->isFullscreen())
        {
            // Hide the cursor
            _view->setCursor(Qt::BlankCursor);
        }

        _vttimer.stop();
        _vthidden = true;
        _playh = false;
    }

    _mpos = cpos;
}


void
GUI_ViewerPane::setPresentationMode(bool pmode, bool showOverlayTimeline)
{
    _presentationMode = pmode;

    setToolbarVisible(false);

    if (showOverlayTimeline)
    {
        QVariantMap tlmd;
        tlmd.insert("toggled", false);
        toggleOverlayTimeline(tlmd);
    }
    
    update();
}


void
GUI_ViewerPane::dragEnterEvent(QDragEnterEvent* event)
{
    // RLP_LOG_DEBUG(event->mimeData()->formats())

    event->setAccepted(true);

    if (event->mimeData()->hasFormat("text/filename") ||
        event->mimeData()->hasFormat("text/plain") ||
        event->mimeData()->hasFormat("text/uri-list"))
    {
        _loadOverlay->setVisible(true);
        _loadOverlay->update();
    }

}


void
GUI_ViewerPane::dragLeaveEvent(QDragLeaveEvent* event)
{
    _loadOverlay->setVisible(false);
}


void
GUI_ViewerPane::dragMoveEvent(QDragMoveEvent* event)
{
    GUI_ItemBase::dragMoveEvent(event);

    _loadOverlay->setMousePos(event->position());
}


void
GUI_ViewerPane::dropEvent(QDropEvent* event)
{
    _loadOverlay->setVisible(false);

    QString path = "";
    if (event->mimeData()->hasFormat("text/filename"))
    {
        path = QString(event->mimeData()->data("text/filename"));
        // RLP_LOG_DEBUG(path)

    } else if (event->mimeData()->hasFormat("text/plain"))
    {
        path = QString(event->mimeData()->data("text/plain"));

        path = path.replace("file://", "");
        path = path.replace("\r", "");
        path = path.replace("\n", "");
        path = path.replace("%20", " ");

    } else if (event->mimeData()->hasFormat("text/uri-list"))
    {
        path = QString(event->mimeData()->data("text/uri-list"));

        path = path.replace("file:///", "");
        path = path.replace("\r", "");
        path = path.replace("\n", "");
        path = path.replace("%20", " ");
    }


    if (path == "")
    {
        return;
    }

    RLP_LOG_DEBUG("Drop Path:" << path)

    DS_TrackPtr tr = _loadOverlay->requestedTrack();
    if (tr == nullptr)
    {
        RLP_LOG_ERROR("Invalid track, aborting")
        return;
    }

    int action = _loadOverlay->requestedAction();

    if (action == 0) // load
    {
        qlonglong loadFrame = tr->getMaxFrameNum() + 1;

        QVariantList mediaInputList;
        mediaInputList.append(path);

        QVariantList argList;
        argList.append(QVariant(mediaInputList));
        argList.append(tr->index());
        argList.append(loadFrame);
        argList.append(false);

        PY_Interp::call("revlens.media.load_media_list", argList);
    
    } else
    {
        RLP_LOG_DEBUG("COMPARE")
        // compare
        CNTRL_MainController* cntrl = CNTRL_MainController::instance();

        DS_SessionPtr sess = CNTRL_MainController::instance()->session();
        DS_NodePtr selNode = tr->getNodeByFrame(cntrl->currentFrameNum());
        if (selNode != nullptr)
        {
            QVariantList argList;
            argList.append(path);
            argList.append(selNode->graph()->uuid().toString());

            QVariantMap callInfo;
            callInfo.insert("method", "revlens.media.append_to_composite");
            callInfo.insert("args", argList); // append(QList) extends list

            RLP_LOG_DEBUG(argList)

            
            
            PY_Interp::call("revlens.media.append_to_composite", argList);
        }
    }
}


void
GUI_ViewerPane::onPlayPausePressed(QVariantMap /* metadata */)
{
    RLP_LOG_INFO("")

    CNTRL_MainController* cntrl = qobject_cast<CNTRL_MainController*>(
        CNTRL_MainController::instance()
    );

    cntrl->runCommand("TogglePlayState");

    _playButton->update();
}


void
GUI_ViewerPane::onPrevFrame(QVariantMap /* metadata */)
{
    RLP_LOG_DEBUG("")

    CNTRL_MainController* cntrl = CNTRL_MainController::instance();
    cntrl->gotoFrame(cntrl->currentFrameNum() - 1);

}


void
GUI_ViewerPane::onNextFrame(QVariantMap /* metadata */)
{
    RLP_LOG_DEBUG("")

    CNTRL_MainController* cntrl = CNTRL_MainController::instance();
    cntrl->gotoFrame(cntrl->currentFrameNum() + 1);

}


void
GUI_ViewerPane::onStartFrame(QVariantMap /* metadata */)
{
    RLP_LOG_DEBUG("")

    CNTRL_MainController* cntrl = CNTRL_MainController::instance();
    cntrl->gotoFrame((qlonglong)1);

}


void
GUI_ViewerPane::onEndFrame(QVariantMap /* metadata */)
{
    RLP_LOG_DEBUG("")

    CNTRL_MainController* cntrl = CNTRL_MainController::instance();
    cntrl->gotoFrame(cntrl->session()->frameCount());

}


void
GUI_ViewerPane::onMuteToggled(QVariantMap md)
{
    RLP_LOG_DEBUG(md);

    CNTRL_Audio* audioCntrl = CNTRL_MainController::instance()->getAudioManager();

    bool isToggled = md.value("toggled").toBool();
    int button = md.value("button").toInt();

    RLP_LOG_DEBUG("Got button:" << button)
    if (button == 2)
    {
        bool vis = !_volButton->menu()->isVisible();
        QPointF mpos = _volButton->mapToItem(
            dynamic_cast<QQuickItem*>(_scene), QPointF(15, 15));

        _volButton->menu()->setPos(mpos);
        _volButton->menu()->setVisible(vis);

        return;
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
GUI_ViewerPane::onVolumeChangeRequested(float val)
{
    RLP_LOG_DEBUG(val)

    CNTRL_Audio* audioCntrl = CNTRL_MainController::instance()->getAudioManager();
    audioCntrl->setVolume(val);

    QString msg = QString("Set Volume: %1").arg(val);

    QVariantList args;
    args.append(msg);
    PY_Interp::call("revlens.cmds.display_message", args);
}


void
GUI_ViewerPane::onReceiveVolumeChange(float val)
{
    RLP_LOG_DEBUG(val)

    QString msg= "";
    if (val == 0.0)
    {
        _volButton->offButton()->setVisible(false);
        _volButton->onButton()->setVisible(true);

        msg = "Mute";
    } else
    {
        _volButton->onButton()->setVisible(false);
        _volButton->offButton()->setVisible(true);

        msg = "Unmute";
    }

    QVariantList args;
    args.append(msg);
    PY_Interp::call("revlens.cmds.display_message", args);
}


void
GUI_ViewerPane::onViewerToolsToggled(QVariantMap md)
{
    bool toggled = _viewerToolsButton->isToggled();
    if (toggled)
    {
        _viewerToolsButton->icon()->setOutlined(false);
        _viewerToolsButton->setToggled(false);

    } else
    {
        RLP_LOG_DEBUG("turning on toggled")
        _viewerToolsButton->icon()->setOutlined(true);
        _viewerToolsButton->setToggled(true);
    }

    _viewerToolsButton->icon()->update();

    // if (_metadata.contains("controls"))
    // {
        // RLP_LOG_DEBUG("GOT CONTROLS")
        // RLP_LOG_DEBUG(_metadata)
        // GUI_ItemBase* controls = _metadata.value("controls").value<GUI_ItemBase*>();
        // RLP_LOG_DEBUG(controls)
        // controls->setVisible(true);
    // }
    
    // DS_EventPlugin* eplug = _view->getEventPluginByName("Composite");
    // if (eplug != nullptr)
    // {
    //     RLP_LOG_DEBUG("GOT EVENT PLUGIN:" << eplug)

    //     QMetaObject::invokeMethod(
    //         eplug,
    //         "setControlsEnabled",
    //         Qt::AutoConnection,
    //         Q_ARG(bool, !toggled)
    //     );
        
    // }
}


void
GUI_ViewerPane::onNoticeEvent(QString evtName, QVariantMap evtData)
{
    if (evtName == "view_request.overlay_timeline")
    {
        RLP_LOG_DEBUG(evtName << evtData)

        QString reqType = evtData.value("method").toString();
        if (reqType == "setFrameInOut")
        {
            qlonglong frameIn = evtData.value("frame_in").toLongLong();
            qlonglong frameOut = evtData.value("frame_out").toLongLong();

            _viewerTimeline->setFrameInOut(frameIn, frameOut);
        }
        else if (reqType == "setFrameNumberStyle")
        {
            int fnumStyle = evtData.value("fnum_style").toInt();
            _viewerTimeline->setFrameNumberStyle(fnumStyle);
        }

        else if (reqType == "setVisible")
        {
            bool isVis = evtData.value("visible").toBool();
            RLP_LOG_DEBUG("Setting visibility of overlay timeline:" << isVis)

            _vtbutton->icon()->setOutlined(isVis);
            _vtbutton->setToggled(isVis);
            _vtbutton->icon()->update();
        }
    }
}


void
GUI_ViewerPane::onParentVisibilityChanged(QString name, bool isVisible)
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
GUI_ViewerPane::onParentSizeChanged(qreal width, qreal height)
{
    if (width == 0) return;
    if (height == 0) return;

    GUI_ItemBase* parent = qobject_cast<GUI_ItemBase*>(parentItem());
    qreal parentHeightOffset = parent->heightOffset();

    setWidth(width + 3); // offset to help prevent white unpainted region
    setHeight(height);

    int hoffset = TOOLBAR_HEIGHT;
    if (!_showToolbar)
    {
        hoffset = 0;
    }

    _view->setWidth(width);
    _view->setHeight(height - hoffset - parentHeightOffset);
    _view->onParentSizeChanged(width, height - hoffset - parentHeightOffset);

    _loadOverlay->setWidth(width - 5);
    _loadOverlay->setHeight(height - hoffset - parentHeightOffset);

    // _vtimeline->setWidth(width);
    // _vtimeline->setHeight(height - hoffset - parentHeightOffset);

    if (_showToolbar)
    {
        _toolbar->setWidth(width + 1); // offset to help prevent white unpainted region
        _toolbar->setHeight(TOOLBAR_HEIGHT + 1);
        _toolbar->setPos(0, height - hoffset - parentHeightOffset - 1);
        _toolbar->onParentSizeChanged(width + 1, height);
    } else {
        _toolbar->setHeight(0);
    }
}


QRectF
GUI_ViewerPane::boundingRect() const
{
    return QRectF(0, 0, width(), height());
}


