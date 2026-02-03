

#include "RlpRevlens/GUI_VIEW/Pane.h"
#include "RlpRevlens/GUI_VIEW/ToolbarManager.h"

#include "RlpGui/BASE/ToolGroup.h"
#include "RlpGui/BASE/Scene.h"
#include "RlpGui/WIDGET/Slider.h"

#include "RlpCore/PY/Interp.h"


#include "RlpRevlens/CNTRL/MainController.h"
#include "RlpRevlens/CNTRL/Video.h"
#include "RlpRevlens/CNTRL/Audio.h"


RLP_SETUP_LOGGER(revlens, GUI_VIEW, Pane)


GUI_VIEW_Pane::GUI_VIEW_Pane(GUI_ItemBase* parent):
    GUI_ItemBase(parent),
    _presentationMode(false),
    _tbYPosOffset(0),
    _tbYHeightOffset(0)
{
    setFlag(QQuickItem::ItemAcceptsDrops);

    // RLP_LOG_DEBUG(parent->heightOffset())

    _view = new DISP_GLView(this);
    
    _loadOverlay = new GUI_VIEW_LoadOverlay(this);
    _loadOverlay->setVisible(false);

    CNTRL_MainController* cntrl = CNTRL_MainController::instance();
    CNTRL_Video* vmgr = cntrl->getVideoManager();
    vmgr->registerDisplay(_view);

    connect(
        _view,
        &DISP_GLView::displayDestroyed,
        vmgr,
        &CNTRL_Video::deregisterDisplay
    );

    connect(
        _view->contextMenu(),
        &GUI_MenuPane::menuItemSelected,
        this,
        &GUI_VIEW_Pane::onViewContextMenuItemSelected
    );

    #ifndef SCAFFOLD_IOS

    // connect(
    //     _view,
    //     &DISP_GLView::hoverEnter,
    //     this,
    //     &GUI_VIEW_Pane::onViewHoverEnter
    // );

    // connect(
    //     _view,
    //     &DISP_GLView::hoverLeave,
    //     this,
    //     &GUI_VIEW_Pane::onViewHoverLeave
    // );

    #endif

    // connect(
    //     _view,
    //     &DISP_GLView::hoverMove,
    //     this,
    //     &GUI_VIEW_Pane::onViewHoverMove
    // );


    setPaintBackground(false);

    onParentSizeChanged(parent->width(), parent->height());

    initToolbars();
}


GUI_VIEW_Pane::~GUI_VIEW_Pane()
{
    delete _view;
}


void
GUI_VIEW_Pane::initToolbars()
{
    // Add filler
    //
    GUI_VIEW_Filler* filler = new GUI_VIEW_Filler(this);
    filler->setHeight(0);
    filler->setWidth(width() + 1);
    filler->setMetadata("filler", true);
    filler->setMetadata("alignment", "bottom");
    filler->setVisible(false);

    _toolbarMap.insert("_filler", filler);
    _visibleToolbars.append(filler);


    RLP_LOG_DEBUG("")
    QVariantMap allTbInfo = GUI_VIEW_ToolbarManager::instance()->getViewerToolbarInfo();

    GUI_MenuPane* tbMenu = _view->contextMenu();

    GUI_MenuItem* tbHeader = tbMenu->addItem("Toolbars");
    tbHeader->setSelectable(false);

    PY_Interp::acquireGIL();
    {
        py::object me = py::cast(this);

        QVariantMap::iterator it;
        for (it = allTbInfo.begin(); it != allTbInfo.end(); ++it)
        {
            QString name = it.key();
            QVariantMap tbInfo = it.value().toMap();

            RLP_LOG_DEBUG(name)

            QString callbackNs = tbInfo.value("callback_ns").toString();

            py::object cbMod = py::module_::import(callbackNs.toLocal8Bit().constData());
            py::object cbObj = cbMod.attr("createToolbar");
            py::object result = cbObj(me);
            
            result.inc_ref();
            if (!result.is(py::none()))
            {
                GUI_ItemBase* toolbar = result.cast<GUI_ItemBase*>();
                toolbar->setMetadata("alignment", tbInfo.value("alignment"));
                toolbar->setVisible(false);

                RLP_LOG_DEBUG(name << toolbar)
                _toolbarMap.insert(name, toolbar);

                QVariantMap tbMd = tbInfo;
                tbMd.insert("type", "toolbar");
                tbMenu->addItem(name, tbMd, /* checkable = */ true);
            }

        }
    }
    PY_Interp::releaseGIL();

    // on-vis playbar by default
    if (_toolbarMap.contains("Playbar"))
    {
        GUI_MenuItem* pbItem = _view->contextMenu()->item("Playbar");
        if (pbItem != nullptr)
        {
            pbItem->selectItem();
        }
    }
}


QQuickItem*
GUI_VIEW_Pane::create(GUI_ItemBase* parent, QVariantMap toolInfo)
{
    GUI_VIEW_Pane* vp = new GUI_VIEW_Pane(parent);

    QVariant v;
    v.setValue(vp->view());

    vp->setMetadata("view", v);

    CNTRL_MainController::instance()->emitToolCreated(QString("Viewer"), vp);

    return vp;
}


void
GUI_VIEW_Pane::onViewContextMenuItemSelected(QVariantMap cmiInfo)
{
    // RLP_LOG_DEBUG(cmiInfo)

    QVariantMap data = cmiInfo.value("data").toMap();
    QString dataType = data.value("type").toString();
    if (dataType == "toolbar")
    {
        QString tbName = cmiInfo.value("text").toString();
        if (_toolbarMap.contains(tbName))
        {
            GUI_ItemBase* toolbar = _toolbarMap.value(tbName);
            GUI_MenuItem* item = cmiInfo.value("item").value<GUI_MenuItem*>();
            if (item->isChecked())
            {
                if (!_visibleToolbars.contains(toolbar))
                {
                    _visibleToolbars.append(toolbar);
                }
            } else
            {
                _visibleToolbars.removeAll(toolbar);
            }

            onParentSizeChanged(width(), height());
        }
    }

    // QString method = cmiInfo.value("data").toMap().value("callback").toString();
    // PY_Interp::call(method, QVariantList());
}



/*

void
GUI_VIEW_Pane::onViewHoverEnter()
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
GUI_VIEW_Pane::onViewHoverLeave()
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
GUI_VIEW_Pane::onViewHoverMove()
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
GUI_VIEW_Pane::toggleOverlayTimeline(QVariantMap md)
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
GUI_VIEW_Pane::checkHideOverlayTimeline()
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
*/


void
GUI_VIEW_Pane::setPresentationMode(bool pmode, bool showTimeline)
{
    _presentationMode = pmode;


    // setToolbarVisible(false);

    // if (showOverlayTimeline)
    // {
    //     QVariantMap tlmd;
    //     tlmd.insert("toggled", false);
    //     toggleOverlayTimeline(tlmd);
    // }

    update();
}


void
GUI_VIEW_Pane::setFillerHeight(int height)
{
    RLP_LOG_DEBUG(height)

    GUI_ItemBase* filler = _toolbarMap.value("_filler");
    if (height == 0)
    {
        filler->setHeight(0);
        filler->setVisible(false);
    } else
    {
        filler->setHeight(height);
        filler->setVisible(true);
    }
}


void
GUI_VIEW_Pane::dragEnterEvent(QDragEnterEvent* event)
{
    RLP_LOG_DEBUG(event->mimeData()->formats())

    if (_view->videoSourceIdx() != 1)
    {
        RLP_LOG_WARN("not a media video source")
        return;
    }
    // QByteArray d = event->mimeData()->data("text/uri-list");
    // RLP_LOG_DEBUG(d)
    // RLP_LOG_DEBUG(event->mimeData()->formats())

    event->setAccepted(true);

    if (event->mimeData()->hasFormat("text/filename") ||
        event->mimeData()->hasFormat("text/plain") ||
        event->mimeData()->hasFormat("text/uri-list"))
    {
        RLP_LOG_DEBUG("DRAG ENTER")

        _loadOverlay->setVisible(true);
        _loadOverlay->update();
    }
}


void
GUI_VIEW_Pane::dragLeaveEvent(QDragLeaveEvent* event)
{
    if (_view->videoSourceIdx() != 1)
    {
        RLP_LOG_WARN("not a media video source")
        return;
    }

    _loadOverlay->setVisible(false);
}


void
GUI_VIEW_Pane::dragMoveEvent(QDragMoveEvent* event)
{
    GUI_ItemBase::dragMoveEvent(event);

    if (_view->videoSourceIdx() != 1)
    {
        RLP_LOG_WARN("not a media video source")
        return;
    }

    _loadOverlay->setMousePos(event->position());
}


void
GUI_VIEW_Pane::dropEvent(QDropEvent* event)
{
    RLP_LOG_DEBUG(event->mimeData()->formats())

 
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


/*
void
GUI_VIEW_Pane::onNoticeEvent(QString evtName, QVariantMap evtData)
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

*/


void
GUI_VIEW_Pane::updateToolbarVis()
{
    // RLP_LOG_DEBUG("")

    GUI_ItemBase* parent = qobject_cast<GUI_ItemBase*>(parentItem());
    qreal parentHeightOffset = parent->heightOffset();

    // Magic, not sure why the viewer has this problem and the other tools dont
    if (parentHeightOffset > 0)
    {
        parentHeightOffset += 5;
    }

    int ty = 0;
    int by = height() - parentHeightOffset + 1; // _view->height();
    int cwidth = width() + 1;

    int yposOffset = 0;
    int yheightOffset = 0;

    ToolbarMapIterator it;
    for (it = _toolbarMap.begin(); it != _toolbarMap.end(); ++it)
    {
        GUI_ItemBase* tb = it.value();
        tb->setWidth(cwidth);
        if (!tb->hasMetadataValue("filler"))
        {
            tb->setVisible(false);
        }
    }


    for (int i = 0; i != _visibleToolbars.size(); ++i)
    {
        GUI_ItemBase* tb = _visibleToolbars.at(i);
        QString tbAlignment = tb->metadataValue("alignment").toString();

        if (tbAlignment == "top")
        {
            tb->setPos(0, ty);
            tb->setVisible(true);

            ty += tb->height();
            yposOffset += tb->height();
            yheightOffset += tb->height();
        }
        else if (tbAlignment == "bottom")
        {
            tb->setPos(0, by - tb->height());
            tb->setVisible(true);

            by -= tb->height();
            yheightOffset += tb->height();

        } else
        {
            RLP_LOG_ERROR("no alignment for toolbar")
        }
    }

    // ??? WTF? Magic
    #ifdef SCAFFOLD_IOS
    if (yposOffset > 0)
    {
        yposOffset -= 20;
        yheightOffset -= 20;
    }
    #endif

    _tbYPosOffset = yposOffset;
    _tbYHeightOffset = yheightOffset;

    emit toolbarVisUpdated();
}


void
GUI_VIEW_Pane::onParentVisibilityChanged(QString name, bool isVisible)
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
GUI_VIEW_Pane::onParentSizeChanged(qreal width, qreal height)
{
    if (width == 0) return;
    if (height == 0) return;

    GUI_ItemBase* parent = qobject_cast<GUI_ItemBase*>(parentItem());
    qreal parentHeightOffset = parent->heightOffset();

    // Magic, not sure why the viewer has this problem and the other tools dont
    if (parentHeightOffset > 0)
    {
        parentHeightOffset += 5;
    }

    setWidth(width); //  + 3); // offset to help prevent white unpainted region
    setHeight(height);

    updateToolbarVis();

    /// RLP_LOG_DEBUG("tbpos offset:" << _tbYPosOffset << "hoffset:" << _tbYHeightOffset << "phoff:" << parentHeightOffset)

    if (_tbYPosOffset != 0)
    {
        int ypos =  _tbYPosOffset - parent->heightOffset(); // WTF? WHY?
        _view->setPos(0, ypos);
        _loadOverlay->setPos(0, _tbYPosOffset);

    } else
    {
        _view->setPos(0, 0);
        _loadOverlay->setPos(0, 0);
    }
    
    int vheight = height - parentHeightOffset - _tbYHeightOffset + 1;

    _view->setWidth(width);
    _view->setHeight(vheight);
    _view->onParentSizeChanged(width, vheight);


    _loadOverlay->setWidth(width);
    _loadOverlay->setHeight(vheight);

    // _vtimeline->setWidth(width);
    // _vtimeline->setHeight(height - hoffset - parentHeightOffset);

}


QRectF
GUI_VIEW_Pane::boundingRect() const
{
    return QRectF(0, 0, width(), height());
}


