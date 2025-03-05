

#include "RlpGui/BASE/Scene.h"


#include "RlpRevlens/DISP/GLView.h"
#include "RlpRevlens/DISP/GLPaintEngine2D.h"


RLP_SETUP_LOGGER(revlens, DISP, GLView)


DISP_GLView::DISP_GLView(GUI_ItemBase* parent, int width, int height):
    GUI_GLItem(parent),
    _uuidStr(QUuid::createUuid().toString()),
    _enabled(true),
    _fullscreen(false),
    _soloPluginIdx(-1),
    _frameRateTimer(),
    _readPixels(false),
    _engine(nullptr)
{
    setAcceptHoverEvents(true);
    // setAcceptTouchEvents(true); TODO: wasm not distinguishing pointer from finger. revisit

    setFlag(QQuickItem::ItemClipsChildrenToShape, true);

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

    _wpr = scene()->view()->devicePixelRatio();

    // TODO FIXME: TEMP FOR WASM TESTING
    //
    // QImage q("/home/justinottley/Videos/big_buck_bunny_trailer_480p/bbb_trailer_480p.0110.jpg");

    /*
    QImage q(":frame.jpg");
    memcpy(_currFrame->data(), q.bits(), _currFrame->getDataBytes());
    
    dynamic_cast<DS_FrameBuffer*>(_currFrame.get())->setColourSpace(
        DS_FrameBuffer::COLSPACE_NONLINEAR);
    
    dynamic_cast<DS_FrameBuffer*>(_currFrame.get())->setChannelOrder(
        DS_FrameBuffer::CHANNEL_ORDER_BGRA);
    */

    // CNTRL_MainController::instance()->registerDisplay(this);
}


DISP_GLView::~DISP_GLView()
{
    RLP_LOG_DEBUG("Num plugins:" << _plugins.size())

    // CNTRL_MainController::instance()->deregisterDisplay(this);

    emit displayDestroyed(this);

    while (_plugins.size() > 0)
    {
        DS_EventPlugin* p = _plugins.takeAt(0);
        if (p->canDuplicate())
        {
            RLP_LOG_DEBUG("Destructing" << p)
            delete p;
        }

    }
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
DISP_GLView::setFrameBuffer(DS_FrameBufferPtr fbuf)
{
    // RLP_LOG_DEBUG("")

    _currFrame = fbuf;
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

    if (_soloPluginIdx >= 0) {
        _plugins[_soloPluginIdx]->hoverMoveEventInternal(event);

    } else {
        for (int i=0; i != _plugins.size(); ++i) {
            if (_plugins[i]->isEnabled()) {
                bool result = _plugins[i]->hoverMoveEventInternal(event);
                if (result) {
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
    
    if (_soloPluginIdx >= 0) {
        _plugins[_soloPluginIdx]->mousePressEventInternal(event);

    } else {
        for (int i=0; i != _plugins.size(); ++i) {
            if (_plugins[i]->isEnabled()) {
                bool result = _plugins[i]->mousePressEventInternal(event);
                if (result) {
                    break;
                }
            }
        }
    }
}


void
DISP_GLView::mouseMoveEvent(QMouseEvent* event)
{
    // RLP_LOG_DEBUG("")

    if (_soloPluginIdx >= 0) {
        _plugins[_soloPluginIdx]->mouseMoveEventInternal(event);

    } else {
        for (int i=0; i != _plugins.size(); ++i) {
            if (_plugins[i]->isEnabled()) {
                bool result = _plugins[i]->mouseMoveEventInternal(event);
                if (result) {
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

    if (_soloPluginIdx >= 0) {
        _plugins[_soloPluginIdx]->mouseReleaseEventInternal(event);

    } else {
        for (int i=0; i != _plugins.size(); ++i) {
            if (_plugins[i]->isEnabled()) {
                bool result = _plugins[i]->mouseReleaseEventInternal(event);
                if (result) {
                    break;
                }
            }
        }
    }
}


bool
DISP_GLView::registerEventPlugin(DS_EventPlugin* plugin)
{
    // RLP_LOG_DEBUG(plugin->name())

    plugin->setDisplay(this);
    _plugins.push_back(plugin);

    return true;
}


DS_EventPlugin*
DISP_GLView::getEventPluginByName(QString name)
{
    for (int i=0; i!= _plugins.size(); ++i)
    {
        DS_EventPlugin* eplugin = _plugins.at(i);
        if (eplugin->name() == name)
        {
            return eplugin;
        }
    }
    return nullptr;
}


DISP_GLPaintEngine2D*
DISP_GLView::getCurrentPaintEngine()
{
    return _engine;

    /*
    if (_currFrame->getBufferMetadata()->contains("display.paintengine")) {
        
        // RLP_LOG_DEBUG("USING PAINT ENGINE FROM CURRENT FRAME BUFFER");
        return _paintEngineMap.value(_currFrame->getBufferMetadata()
                                               ->value("display.paintengine").
                                               value<QString>());
    }

    QString defaultPaintEngine("Default2D");
    if (_paintEngineMap.contains(defaultPaintEngine)) {
        return _paintEngineMap.value(defaultPaintEngine);
    }

    return nullptr;
    */
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
        DISP_GLPaintEngine2D* eng = func();

        _paintEngineMap.insert(name, eng);

        emit paintEngineCreated(eng);

        return eng;
    }

    RLP_LOG_WARN("Not found:" << name);
    return nullptr;
}



void
DISP_GLView::setSoloPluginByIdx(int idx)
{
    RLP_LOG_INFO("Setting Solo plugin: index " << idx);
    _soloPluginIdx = idx;
}


void
DISP_GLView::setSoloPluginByName(QString name)
{
     for (int i = 0; i != _plugins.size(); ++i) {
         if (_plugins[i]->name() == name) {
             
             RLP_LOG_INFO("Setting Solo plugin: " << name);
             _soloPluginIdx = i;
             _plugins[i]->setEnabled(true);
         }
     }
}


void
DISP_GLView::resetSoloPlugin(QString name, bool enabled)
{
    // RLP_LOG_INFO("");

    if ((_soloPluginIdx != -1) &&
        (_plugins[_soloPluginIdx]->name() == name))
    {
        _plugins[_soloPluginIdx]->setEnabled(enabled);
        _soloPluginIdx = -1;
    }
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
        return _engine->getZoom();
    }

    return 1.0;
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
    for (it=_paintEngineMap.begin(); it != _paintEngineMap.end(); ++it) {
        (*it)->setSize(width, height);
    }

    if (_engine != nullptr)
    {
        _engine->resizeGLInternal(width, height);
    }

    for (int i = 0; i != _plugins.size(); ++i) {
    
        // RLP_LOG_DEBUG(_plugins[i]->name());
        _plugins[i]->resizeGLInternal(width, height);
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

        DISP_GLPaintEngine2D* e = getOrInitPaintEngine(_currPaintEngine);

        // _metadata.insert(e->metadata());

        e->setSize(width(), height());

        connect(
            e,
            SIGNAL(displayZoomChanged(float)),
            this,
            SLOT(onPaintEngineDisplayZoomChanged(float))
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
    }

    // RLP_LOG_DEBUG(isVisible())

    // RLP_LOG_DEBUG("Window Pixel Aspect Ratio: " << _wpr)
    // RLP_LOG_DEBUG(_currFrame->getWidth() << _currFrame->getHeight() << _currFrame->getDataBytes())

    _engine->initializeGLInternal(_currFrame);
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
    glScissor(sx, sy, w + 10, h + 1); // +10 offset to help prevent white unpainted region of screen
    glViewport(sx, sy, w, h + 1);

    glDisable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    // glClearColor(0.87, 0.07, 0.17, 1.0);
    glClearColor(0.07, 0.07, 0.17, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /*
    _engine->setScreenRect(_srect);
    _engine->paintGLInternal(_currFrame);
    */

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

    scene()->view()->endExternalCommands();
}


void
DISP_GLView::paint(GUI_Painter* painter)
{
    if (!_enabled)
    {
        return;
    }
    // GUI_GLItem::paint(painter);

    QVariantMap metadataMap; //  = _metadata;

    QVariant fbufWrapper;
    fbufWrapper.setValue(_currFrame);

    metadataMap.insert("video.currframe", (qlonglong)_currFrame->getFrameNum());
    metadataMap.insert("video.currfps", _frameRateTimer.fpsAverage());
    metadataMap.insert("video.framebuffer", fbufWrapper);

    metadataMap.insert("window.height", height());
    metadataMap.insert("window.width", width());

    metadataMap.insert("display.zoom", getZoom());

    for (int i = 0; i != _plugins.size(); ++i) {
        _plugins[i]->paintGLPostDraw(painter, metadataMap);
    }
    
    _frameRateTimer.time();
}


void
DISP_GLView::grabImage()
{
    _readPixels = true;
    update();
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