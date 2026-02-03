

#include "RlpGui/BASE/Scene.h"
#include "RlpGui/PANE/View.h"

#include "RlpRevlens/DISP/GLView.h"
#include "RlpRevlens/DISP/GLPaintEngine2D.h"


RLP_SETUP_LOGGER(revlens, DISP, GLView)


DISP_GLView::DISP_GLView(GUI_ItemBase* parent, int width, int height):
    GUI_GLItem(parent),
    _uuidStr(QUuid::createUuid().toString(QUuid::WithoutBraces)),
    _idx(-1),
    _videoSourceIdx(1),
    _enabled(true),
    _drawEnabled(true),
    _fullscreen(false),
    _compositePlugins(nullptr),
    _soloPluginName(""),
    _frameRateTimer(),
    _readPixels(false),
    _engine(nullptr)
{
    setAcceptHoverEvents(true);
    setAcceptTouchEvents(true); //  TODO: wasm not distinguishing pointer from finger. revisit

    setFlag(QQuickItem::ItemClipsChildrenToShape, true);

    GUI_Scene* myScene = scene();

    connect(
        myScene,
        &GUI_Scene::tabletEventInternal,
        this,
        &DISP_GLView::onTabletEvent
    );

    // Turn on on these sig-slot connections to have the display not draw when resizing panels
    // It seems on linux panel resizing can be slow, but it appears to not be helped by turning
    // off the display, it seems to be specific to some tools
    //
    // connect(
    //     myScene,
    //     &GUI_Scene::splitterDragStart,
    //     this,
    //     &DISP_GLView::onSplitterDragStarted
    // );

    // connect(
    //     myScene,
    //     &GUI_Scene::splitterDragEnd,
    //     this,
    //     &DISP_GLView::onSplitterDragEnded
    // );

    _displayMetadata = new QVariantMap();
    _displayMetadata->insert("display.uuid", _uuidStr);

    QVariant objWrapper;
    objWrapper.setValue(this);
    _displayMetadata->insert("display.obj", objWrapper);

    _contextMenu = new GUI_MenuPane(this);
    _contextMenu->setVisible(false);

    _plugins.clear();
    _peInitMap.clear();
    _paintEngineMap.clear();


    QVariantMap frameInfo;
    
    int defaultChannelCount = 4;
    int defaultPixelTypeDepth = 1; // sizeof(half);
    // int defaultPixelSize = defaultChannelCount * defaultPixelTypeDepth;
    
    frameInfo.insert("width", 0);
    frameInfo.insert("height", 0);
    frameInfo.insert("channelCount", defaultChannelCount);
    frameInfo.insert("pixelSize", defaultPixelTypeDepth);
    
    _currFrame = std::shared_ptr<DS_FrameBuffer>(new DS_FrameBuffer(this, frameInfo));
    _currFrame->reallocate();
    _currFrame->setFrameNum(0);

    GUI_View* view = qobject_cast<GUI_View*>(scene()->view());

    _wpr = view->devicePixelRatio();

    connect(
        view,
        &GUI_View::screenChanged,
        this,
        &DISP_GLView::setDevicePixelRatio
    );
}


DISP_GLView::~DISP_GLView()
{
    RLP_LOG_DEBUG("Num plugins:" << _plugins.size())

    emit displayDestroyed(this);

    QStringList pluginKeys = _plugins.keys();
    for (int i = 0; i != pluginKeys.size(); ++i)
    {
        QString plugName = pluginKeys.at(i);
        if (_plugins.value(plugName)->canDuplicate())
        {
            RLP_LOG_DEBUG("Destructing Plugin" << plugName)
            DS_EventPlugin* plug = _plugins.take(plugName);
            delete plug;
        }

    }
}


void
DISP_GLView::setIdx(int idx)
{
    RLP_LOG_DEBUG(idx)

    _idx = idx;
    _displayMetadata->insert("display.idx", _idx);
}


void
DISP_GLView::setDevicePixelRatio(qreal dpr)
{
    RLP_LOG_DEBUG(dpr)

    _wpr = dpr;
}


void
DISP_GLView::initPaintEngines(PaintEngineCreateMap peMap)
{
    _peInitMap = peMap;

    PaintEngineCreateMap::iterator i;
    for (i = peMap.begin(); i != peMap.end(); ++i)
    {
        RLP_LOG_DEBUG(i.key());
    }
    
}


void
DISP_GLView::setCurrentPaintEngine(QString pe)
{
    RLP_LOG_DEBUG(pe)

    _currPaintEngine = pe;
}


void
DISP_GLView::setFrameBuffer(int videoSourceIdx, DS_FrameBufferPtr fbuf)
{
    // RLP_LOG_DEBUG("")

    _videoSourceFbufMap.insert(videoSourceIdx, fbuf);
}


void
DISP_GLView::setVideoSourceIdx(int videoSourceIdx)
{
    RLP_LOG_DEBUG(videoSourceIdx)

    _videoSourceIdx = videoSourceIdx;
    _currFrame = _videoSourceFbufMap[videoSourceIdx];
}


void
DISP_GLView::setEnabled(bool isEnabled)
{
    RLP_LOG_DEBUG(isEnabled)

    _enabled = isEnabled;
}


void
DISP_GLView::setCompositeMode(QString compositeModeName)
{
    RLP_LOG_DEBUG(compositeModeName)

    _displayMetadata->insert("composite_mode", compositeModeName);

    emit compositeModeChanged(compositeModeName);
}


void
DISP_GLView::setDisplayMetadata(QString key, QVariant val)
{
    // RLP_LOG_DEBUG(key << val)

    _displayMetadata->insert(key, val);

    PluginIterator it;
    for (it = _pluginNames.begin(); it != _pluginNames.end(); ++it)
    {
        DS_EventPlugin* plug = _plugins.value(*it);
        if (plug->isEnabled())
        {
            plug->reset();
        }
    }

    update();
}


void
DISP_GLView::clearDisplayMetadata(QString prefix)
{
    if (prefix == "")
    {
        _displayMetadata->clear();
    } else
    {
        QVariantMap::iterator it;

        QStringList clearKeys;
        for (it = _displayMetadata->begin(); it != _displayMetadata->end(); ++it)
        {
            if (it.key().startsWith(prefix))
            {
                RLP_LOG_DEBUG("Clearing key:" << it.key());
                clearKeys.append(it.key());
            }
        }

        RLP_LOG_DEBUG("Clearing keys:" << clearKeys)
    }
}


void
DISP_GLView::onSplitterDragStarted()
{
    RLP_LOG_DEBUG("")
    _drawEnabled = false;
}


void
DISP_GLView::onSplitterDragEnded()
{
    RLP_LOG_DEBUG("")
    _drawEnabled = true;
}


DS_FrameBufferPtr
DISP_GLView::presentNextFrame(long frameNum)
{
    _currFrame->setFrameNum(frameNum);
    return _currFrame;
}


bool
DISP_GLView::event(QEvent* ievent)
{
    // RLP_LOG_DEBUG(ievent)
    /*
     * TODO: Disabled. wasm is not distinguising pencel from finger. revisit at some point
     *
    QEvent::Type etype = ievent->type();
    if ((etype == QEvent::TouchBegin) ||
        (etype == QEvent::TouchUpdate) ||
        (etype == QEvent::TouchEnd) ||
        (etype == QEvent::TouchCancel))
    {
        RLP_LOG_DEBUG("Masking touch event:" << ievent)
        QTouchEvent* te = static_cast<QTouchEvent*>(ievent);
        RLP_LOG_DEBUG(te->deviceType() << te->pointerType() << te->pointingDevice())
        return true;
    }
    *
    */

    return GUI_GLItem::event(ievent);
}


void
DISP_GLView::hoverEnterEvent(QHoverEvent* event)
{
    // RLP_LOG_DEBUG("")

    // setCursor(Qt::CrossCursor);

    _inHover = true;

    emit hoverEnter();
}


void
DISP_GLView::hoverLeaveEvent(QHoverEvent* event)
{
    // RLP_LOG_DEBUG("")

    setCursor(Qt::ArrowCursor);

    _inHover = false;
    emit hoverLeave();
}


void 
DISP_GLView::hoverMoveEvent(QHoverEvent* event)
{
    // RLP_LOG_DEBUG(event->pos())

    if (_soloPluginName != "")
    {
        _plugins.value(_soloPluginName)->hoverMoveEventInternal(event);

    } else
    {
        PluginIterator it;
        for (it = _pluginNames.begin(); it != _pluginNames.end(); ++it)
        {
            DS_EventPlugin* plug = _plugins.value(*it);
            if (plug->isEnabled())
            {
                bool result = plug->hoverMoveEventInternal(event);
                if (result)
                {
                    break;
                }
            }
        }
    }

    emit hoverMove();
}


void 
DISP_GLView::mousePressEvent(QMouseEvent* event)
{
    
    if (_soloPluginName != "")
    {
        _plugins.value(_soloPluginName)->mousePressEventInternal(event);

    } else
    {
        PluginIterator it;
        for (it = _pluginNames.begin(); it != _pluginNames.end(); ++it)
        {
            DS_EventPlugin* plug = _plugins.value(*it);
            if (plug->isEnabled())
            {
                bool result = plug->mousePressEventInternal(event);
                if (result)
                {
                    break;
                }
            }
        }
    }
}


void
DISP_GLView::mouseMoveEvent(QMouseEvent* event)
{
    // RLP_LOG_DEBUG(event->position() << event->globalPosition())

    if (_soloPluginName != "")
    {
        _plugins.value(_soloPluginName)->mouseMoveEventInternal(event);

    } else
    {
        PluginIterator it;
        for (it = _pluginNames.begin(); it != _pluginNames.end(); ++it)
        {
            DS_EventPlugin* plug = _plugins.value(*it);
            if (plug->isEnabled())
            {
                bool result = plug->mouseMoveEventInternal(event);
                if (result)
                {
                    break;
                }
            }
        }
    }
}


void
DISP_GLView::mouseReleaseEvent(QMouseEvent* event)
{
    // RLP_LOG_DEBUG("")

    if (_soloPluginName != "")
    {
        _plugins.value(_soloPluginName)->mouseReleaseEventInternal(event);

    } else
    {
        PluginIterator it;
        for (it = _pluginNames.begin(); it != _pluginNames.end(); ++it)
        {
            DS_EventPlugin* plug = _plugins.value(*it);
            if (plug->isEnabled())
            {
                bool result = plug->mouseReleaseEventInternal(event);
                if (result)
                {
                    break;
                }
            }
        }
    }
}


void
DISP_GLView::touchEvent(QTouchEvent* event)
{
    // RLP_LOG_DEBUG(event)

    if (_soloPluginName != "")
    {
        _plugins.value(_soloPluginName)->touchEventInternal(event);

    } else
    {
        PluginIterator it;
        for (it = _pluginNames.begin(); it != _pluginNames.end(); ++it)
        {
            DS_EventPlugin* plug = _plugins.value(*it);
            if (plug->isEnabled())
            {
                bool result = plug->touchEventInternal(event);
                if (result)
                {
                    break;
                }
            }
        }
    }
}


void
DISP_GLView::onTabletEvent(QTabletEvent* event)
{
    event->setAccepted(true); // don't synthesize into mouse event

    if (_soloPluginName != "")
    {
        _plugins.value(_soloPluginName)->tabletEventInternal(event);

    } else
    {
        PluginIterator it;
        for (it = _pluginNames.begin(); it != _pluginNames.end(); ++it)
        {
            DS_EventPlugin* plug = _plugins.value(*it);
            if (plug->isEnabled())
            {
                bool result = plug->tabletEventInternal(event);
                if (result)
                {
                    break;
                }
            }
        }
    }
}


void
DISP_GLView::registerCompositePlugins(CompositePluginMap* cpm)
{
    RLP_LOG_DEBUG("")

    CompositePluginMap::iterator it;

    CompositePluginMap* myCpm = _compositePlugins;
    if (myCpm == nullptr)
    {
        myCpm = new CompositePluginMap();
        _compositePlugins = myCpm;
    }

    for (it = cpm->begin(); it != cpm->end(); ++it)
    {
        // RLP_LOG_DEBUG(it.key())

        DISP_CompositePlugin* cp = it.value()->duplicate();
        cp->setDisplay(this);
        myCpm->insert(cp->name(), cp);
    }
}


bool
DISP_GLView::registerEventPlugin(DS_EventPlugin* plugin)
{
    // RLP_LOG_DEBUG(plugin->name())

    plugin->setDisplay(this);
    _plugins.insert(plugin->name(), plugin);
    _pluginNames.push_back(plugin->name());

    return true;
}


bool
DISP_GLView::setPluginIndex(QString name, int idx)
{
    RLP_LOG_DEBUG(name << idx)
    RLP_LOG_DEBUG(_pluginNames)

    if (_pluginNames.contains(name))
    {
        int currIdx = _pluginNames.indexOf(name);
        _pluginNames.takeAt(currIdx);
        _pluginNames.insert(idx, name);
        
        RLP_LOG_DEBUG(_pluginNames)
        return true;
    }

    return false;
}


DS_EventPlugin*
DISP_GLView::getEventPluginByName(QString name)
{
    if (_plugins.contains(name))
    {
        return _plugins.value(name);
    }

    return nullptr;
}


DISP_GLPaintEngine2D*
DISP_GLView::getCurrentPaintEngine()
{
    return _engine;
}


DISP_GLPaintEngine2D*
DISP_GLView::getOrInitPaintEngine(QString name)
{
    // MUST BE CALLED ON RENDERING THREAD
    //

    if (_paintEngineMap.contains(name))
    {
        return _paintEngineMap.value(name);
    }

    if (_peInitMap.contains(name))
    {
        RLP_LOG_DEBUG("Constructing new Paint Engine" << name)

        PaintEngineCreateFunc func = _peInitMap.value(name);
        DISP_GLPaintEngine2D* paintEngine = func();

        paintEngine->setCompositePlugins(_compositePlugins);
        paintEngine->setDisplayMetadata(_displayMetadata);

        _paintEngineMap.insert(name, paintEngine);

        emit paintEngineCreated(paintEngine);

        return paintEngine;
    }

    RLP_LOG_WARN("Not found:" << name);
    return nullptr;
}


bool
DISP_GLView::setSoloPluginByName(QString name)
{
    if (_plugins.contains(name))
    {
        _plugins.value(name)->setEnabled(true);
        _soloPluginName = name;
        return true;
    }

    return false;
}


bool
DISP_GLView::resetSoloPlugin(QString name, bool enabled)
{
    // RLP_LOG_INFO("");

    if ((_soloPluginName != "") &&
        (_plugins.value(_soloPluginName)->name() == name))
    {
        _plugins.value(_soloPluginName)->setEnabled(enabled);
        _soloPluginName = "";
        return true;
    }

    return false;
}


QPointF
DISP_GLView::getPanPercent()
{
    if (_engine != nullptr)
    {
        return _engine->getPanPercent();
    }

    return QPointF(0, 0);
}


float
DISP_GLView::getZoom()
{
    if (_engine != nullptr)
    {
        return _engine->getDisplayZoom() + _engine->getZoom();
    }

    return 1.0;
}


float
DISP_GLView::getDisplayZoom()
{
    if (_engine != nullptr)
    {
        return _engine->getDisplayZoom();
    }

    return 1.0;
}


float
DISP_GLView::getRelZoom()
{
    if (_engine != nullptr)
    {
        return _engine->getZoom();
    }

    return 0.0;
}


void
DISP_GLView::resetZoom()
{
    _engine->resetZoom();
}


void
DISP_GLView::resetPan()
{
    _engine->resetPan();
}


void
DISP_GLView::onParentSizeChanged(qreal rwidth, qreal rheight)
{
    // RLP_LOG_DEBUG(rwidth << " " << rheight)

    if (rwidth == 0)
        return;

    if (rheight == 0)
        return;


    int width = int(rint(rwidth));
    int height = int(rint(rheight));

    PaintEngineMapIterator it;
    for (it=_paintEngineMap.begin(); it != _paintEngineMap.end(); ++it)
    {
        (*it)->setSize(width, height);
    }

    if (_engine != nullptr)
    {
        _engine->resizeGLInternal(width, height);
    }

    PluginIterator pit;
    for (pit=_pluginNames.begin(); pit != _pluginNames.end(); ++pit)
    {
        DS_EventPlugin* plug = _plugins.value(*pit);
        plug->resizeGLInternal(width, height);
    }
}


void
DISP_GLView::initGL()
{
    if (!_enabled)
    {
        return;
    }

    if (!isVisible())
    {
        return;
    }


    // QtQuick GL draw is multithreaded. Paint engine needs to be constructed
    // in initGL() which is called from the rendering thread, since the paint engine
    // needs to have the same thread affinity (it makes GL shader objects)
    //
    if (_engine == nullptr)
    {

        // There are some stability issues with the annotation system attempting to draw (in paintGLPostDraw)
        // seemingly before the display has fully come up (I think). Disable drawing while initializing 
        // in an attempt to improve this
        // 
        _enabled = false;

        DISP_GLPaintEngine2D* e = getOrInitPaintEngine(_currPaintEngine);

        // _metadata.insert(e->metadata());

        e->setSize(width(), height());

        connect(
            e,
            &DISP_GLPaintEngine2D::displayZoomEnd,
            this,
            &DISP_GLView::displayZoomEnd
        );

        connect(
            e,
            &DISP_GLPaintEngine2D::displayPanEnd,
            this,
            &DISP_GLView::displayPanEnd
        );

        connect(
            this,
            &DISP_GLView::panPosChanged,
            e,
            &DISP_GLPaintEngine2D::setPanPos
        );

        connect(
            this,
            &DISP_GLView::zoomPosChanged,
            e,
            &DISP_GLPaintEngine2D::setZoomPos
        );

        connect(
            this,
            &DISP_GLView::onPanZoomEnd,
            e,
            &DISP_GLPaintEngine2D::setPanZoomEnd
        );

        _engine = e;

        _engine->initializeGLInternal(_currFrame);

        // Enable only after running over the event loop
        QTimer::singleShot(10, this, &DISP_GLView::enable);


    } else if (_drawEnabled)
    {
        _engine->initializeGLInternal(_currFrame);
    }

    // RLP_LOG_DEBUG(isVisible())

    // RLP_LOG_DEBUG("Window Pixel Aspect Ratio: " << _wpr)
    // RLP_LOG_DEBUG(_currFrame->getWidth() << _currFrame->getHeight() << _currFrame->getDataBytes())

    
}


void
DISP_GLView::paintGL()
{
    if (!_enabled)
    {
        return;
    }

    if (!isVisible())
    {
        return;
    }


    scene()->view()->beginExternalCommands();

    glEnable(GL_SCISSOR_TEST);

    qreal w = width() * _wpr;
    qreal h = (height()) * _wpr;

    // RLP_LOG_DEBUG(w << h)

    QPointF gpos = mapToScene(position());

    qreal ssh = scene()->height() * _wpr;
    qreal sy = (ssh - ((gpos.y() * _wpr) + h));
    qreal sx = gpos.x() * _wpr;

    _srect = QRect(sx, sy, w, h + 1);

    _displayMetadata->insert("srect", _srect);

    glScissor(sx, sy, w + 10, h + 1); // +10 offset to help prevent white unpainted region of screen
    glViewport(sx, sy, w, h + 1);

    glDisable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    // glClearColor(0.87, 0.07, 0.17, 1.0);
    // glClearColor(0.07, 0.07, 0.17, 1.0);
    glClearColor(0.07, 0.07, 0.07, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    if (_drawEnabled)
    {
        _engine->setScreenRect(_srect);
        _engine->paintGLInternal(_currFrame);


        if (_readPixels)
        {
            // not sure why there are an extra 4 pixels...
            QImage img(w - 4, h, QImage::Format_RGBA8888);
            glReadPixels(sx, sy, w - 4, h, GL_RGBA, GL_UNSIGNED_BYTE, img.bits());

            img = img.mirrored(false, true);

            _readPixels = false;
            
            emit grabImageReady(img);
        }
    }
    
    scene()->view()->endExternalCommands();
}


void
DISP_GLView::paint(GUI_Painter* painter)
{
    if (!_enabled)
    {
        return;
    }

    if (!_drawEnabled)
    {
        return;
    }

    QVariantMap metadataMap;

    QVariant fbufWrapper;
    fbufWrapper.setValue(_currFrame);

    metadataMap.insert("video.currframe", (qlonglong)_currFrame->getFrameNum());
    metadataMap.insert("video.currfps", _frameRateTimer.fpsAverage());
    metadataMap.insert("video.framebuffer", fbufWrapper);

    metadataMap.insert("window.height", height());
    metadataMap.insert("window.width", width());

    metadataMap.insert("display.zoom", getZoom());
    metadataMap.insert(*_displayMetadata);

    PluginIterator it;
    for (it=_pluginNames.begin(); it != _pluginNames.end(); ++it)
    {
        QString plugName = *it;

        DS_EventPlugin* plug = _plugins.value(*it, nullptr);
        if (plug != nullptr)
        {
            plug->paintGLPostDraw(painter, metadataMap);
        }
    }

    _frameRateTimer.time();
}


void
DISP_GLView::grabImage()
{
    _readPixels = true;
    update();
}


QVariantList
DISP_GLView::getCurrentFramebufferList()
{
    QVariantList result;

    DS_FrameBuffer* fbuf = dynamic_cast<DS_FrameBuffer* >(_currFrame.get());
    DS_FrameBuffer::exportFramebufferList(fbuf, &result);

    return result;
}


void
DISP_GLView::enableFullscreen()
{
    _fullscreen = true;
    emit fullscreenEnabled(_fullscreen, _uuidStr);
}


void
DISP_GLView::disableFullscreen()
{
    _fullscreen = false;
    emit fullscreenEnabled(_fullscreen, _uuidStr);
}