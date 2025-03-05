//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include "RlpGui/BASE/Scene.h"

#include "RlpRevlens/CNTRL/Video.h"
#include "RlpExtrevlens/RLQTDISP/Composite.h"


RLP_SETUP_EXT_LOGGER(RLQTDISP, Composite)
RLP_SETUP_EXT_LOGGER(RLQTDISP, CompositeTools)


RLQTDISP_CompositeTools::RLQTDISP_CompositeTools(DISP_GLView* parent):
    GUI_ItemBase(parent),
    _display(parent)
{
    setOpacity(0.75);

    _modeButton = new GUI_IconButton(":misc/layer.png", this, 15);
    //_modeButton->setColour(QColor(120, 120, 120));
    _modeButton->addAction("Tile");
    _modeButton->addAction("Wipe");
    _modeButton->addAction("Blend");

    _modeButton->setText("Tile");
    _modeButton->setTextHOffset(4);
    _modeButton->setTextYOffset(2);
    _modeButton->setOutlined(true);

    connect(
        _modeButton->menu(),
        SIGNAL(menuItemSelected(QVariantMap)),
        this,
        SLOT(onMenuItemSelected(QVariantMap))
    );
}


void
RLQTDISP_CompositeTools::setEnabled(bool enabled)
{
    RLP_LOG_DEBUG(enabled)

    _modeButton->setVisible(enabled);
}


void
RLQTDISP_CompositeTools::onMenuItemSelected(QVariantMap itemInfo)
{
    QString itemName = itemInfo.value("text").toString();

    // RLP_LOG_DEBUG(itemName)

    _modeButton->setText(itemName);

    DS_FrameBufferPtr fbuf = _display->frameBuffer();
    QVariantMap* bm = fbuf->getBufferMetadata();

    if (!bm->contains("node.composite.idx"))
    {
        return;
    }    

    QString nodeUuidStr = bm->value("graph.uuid").toString();

    RLP_LOG_DEBUG("Switching Composite Mode to:" << itemName << "for node:" << nodeUuidStr)

    CNTRL_MainController* controller = CNTRL_MainController::instance();
    DS_NodePtr node = controller->session()->getNodeByUuid(nodeUuidStr);
    node->setProperty("display.composite_mode", itemName);


    // need to eject everything from the lookahead to recache the metadata
    // into the framebuffers
    CNTRL_Video* cv = controller->getVideoManager();

    qlonglong cframe = cv->currentFrameNum();

    RLP_LOG_DEBUG("Current frame:" << cframe)

    cv->clearAll();
    cv->updateToFrame(cframe, true);

    emit compositeModeChanged(itemName);
}


QRectF
RLQTDISP_CompositeTools::boundingRect() const
{
    return _modeButton->boundingRect();
}


void
RLQTDISP_CompositeTools::paint(GUI_Painter* painter)
{
}


RLQTDISP_Composite::RLQTDISP_Composite():
    DS_EventPlugin(QString("Composite"), true),
    _windowWidth(0),
    _windowHeight(0),
    _mode("Tile"),
    _visEnabled(true)
{
    _imgMap.clear();
}


DS_EventPlugin*
RLQTDISP_Composite::duplicate()
{
    RLQTDISP_Composite* childPlug = new RLQTDISP_Composite();
    childPlug->setMainController(_controller);

    _childPlugins.append(childPlug);

    return childPlug;
}


bool
RLQTDISP_Composite::setMainController(CNTRL_MainController* controller)
{
    DS_EventPlugin::setMainController(controller);

    connect(
        controller,
        SIGNAL(sessionChanged(DS_SessionPtr)),
        this,
        SLOT(onSessionChanged(DS_SessionPtr))
    );
    
    connect(
        controller->session().get(),
        SIGNAL(sessionCleared()),
        this,
        SLOT(onSessionCleared())
    );

    connect(
        this,
        &RLQTDISP_Composite::imageWindowNeeded,
        this,
        &RLQTDISP_Composite::onImageWindowNeeded
    );


    return true;
}


bool
RLQTDISP_Composite::setDisplay(DISP_GLView* display)
{
    DS_EventPlugin::setDisplay(display);

    _tools = new RLQTDISP_CompositeTools(display);
    connect(
        _tools,
        SIGNAL(compositeModeChanged(QString)),
        this,
        SLOT(onCompositeModeChanged(QString))
    );

    // TODO FIXME: Shouldn't these GUI widgets be moved to CompositeTools?

    _wsplitter = new RLQTDISP_CompositeWipeSplitter(_display);
    _wsplitter->setPosPercent(0.5);
    _wsplitter->setPos(display->width() * _wsplitter->posPercent(), 0);
    _wsplitter->setVisible(false);

    connect(
        _wsplitter,
        SIGNAL(posChanged(float)),
        this,
        SLOT(onWipePosChanged(float))
    );

    _blendSlider = new GUI_Slider(_display, GUI_Slider::O_SL_HORIZONTAL);
    _blendSlider->setBgColour(GUI_Style::BgLoMidGrey);
    _blendSlider->setOpacity(0.75);
    _blendSlider->setPos(30, 10);
    _blendSlider->setSliderValue(0.5);
    _blendSlider->setVisible(false);

    connect(
        _blendSlider,
        &GUI_Slider::moving,
        this,
        &RLQTDISP_Composite::onBlendPosChanged
    );

    return true;
}


void
RLQTDISP_Composite::onImageWindowNeeded(QVariantMap* bm)
{
    int fbufWidth = bm->value("width").value<int>();
    int fbufHeight = bm->value("height").value<int>();
    int compositeIdx = bm->value("node.composite.idx").value<int>();


    RLQTDISP_CompositeImageWindow* w = new RLQTDISP_CompositeImageWindow(_display, "", fbufWidth, fbufHeight, bm);
    w->setEnabled(_visEnabled);
    _imgMap.insert(compositeIdx, w);
}


void
RLQTDISP_Composite::onCompositeModeChanged(QString compMode)
{
    RLP_LOG_DEBUG(compMode)

    _mode = compMode;

    bool tileVisible = false;
    bool wipeSplitterVisible = false;
    bool blendSliderVisible = false;

    if (compMode == "Tile")
    {
        tileVisible = true;
    } else if (compMode == "Wipe")
    {
        wipeSplitterVisible = true;

    } else if (compMode == "Blend")
    {
        blendSliderVisible = true;
    }


    _wsplitter->setVisible(wipeSplitterVisible);

    QList<int> mapIdxList = _imgMap.keys();
    for (int i=0; i != mapIdxList.size(); ++i)
    {
        int compositeIdx = mapIdxList.at(i);
        _imgMap.value(compositeIdx)->setVisible(tileVisible);
    }

    _blendSlider->setVisible(blendSliderVisible);
}


void
RLQTDISP_Composite::onWipePosChanged(float xpos)
{
    // RLP_LOG_DEBUG(xpos)

    DS_FrameBufferPtr fbuf = _display->frameBuffer();
    QVariantMap* bm = fbuf->getBufferMetadata();

    if (!bm->contains("node.composite.idx"))
    {
        return;
    }

    QString nodeUuidStr = bm->value("graph.uuid").toString();

    CNTRL_MainController* controller = CNTRL_MainController::instance();
    DS_NodePtr node = controller->session()->getNodeByUuid(nodeUuidStr);

    node->setProperty("display.wipe.xpos", xpos);

    for (int fi=0; fi != fbuf->numAuxBuffers(false /* includePrimary */); ++fi)
    {
        DS_BufferPtr afbuf = fbuf->getAuxBuffer(fi);
        afbuf->getBufferMetadata()->insert("display_wipe.xpos", xpos);
    }

    _display->update();

}


void
RLQTDISP_Composite::onBlendPosChanged(float pos)
{
    DS_FrameBufferPtr fbuf = _display->frameBuffer();
    QVariantMap* bm = fbuf->getBufferMetadata();
    QString nodeUuidStr = bm->value("graph.uuid").toString();

    CNTRL_MainController* controller = CNTRL_MainController::instance();
    DS_NodePtr node = controller->session()->getNodeByUuid(nodeUuidStr);

    node->setProperty("display.blend.value", pos);

    bm->insert("alpha", pos);
    for (int fi=0; fi != fbuf->numAuxBuffers(false /* includePrimary */); ++fi)
    {
        DS_BufferPtr afbuf = fbuf->getAuxBuffer(fi);
        afbuf->getBufferMetadata()->insert("alpha", 1 - pos);
    }

    _display->update();

}


void
RLQTDISP_Composite::onSessionChanged(DS_SessionPtr session)
{
    // RLP_LOG_DEBUG("")
    clear();
    _imgMap.clear();
}


void
RLQTDISP_Composite::onSessionCleared()
{
    RLP_LOG_DEBUG("")

    clear();
    _imgMap.clear();
}


void
RLQTDISP_Composite::clear()
{
    RLP_LOG_DEBUG("")

    QList<int> mapIdxList = _imgMap.keys();

    for (int i=0; i != mapIdxList.size(); ++i)
    {
        int compositeIdx = mapIdxList.at(i);
        _imgMap.value(compositeIdx)->deleteLater();
    }

    _imgMap.clear();
}


void
RLQTDISP_Composite::hideTools()
{
    _tools->setVisible(false);
    _wsplitter->setVisible(false);

    QList<int> mapIdxList = _imgMap.keys();
    for (int i=0; i != mapIdxList.size(); ++i)
    {
        _imgMap.value(mapIdxList.at(i))->setEnabled(false);
    }
    _blendSlider->setVisible(false);
}


void
RLQTDISP_Composite::setControlsEnabled(bool enabled)
{
    RLP_LOG_DEBUG(enabled)

    _visEnabled = enabled;

    _tools->setEnabled(enabled);
    _wsplitter->setEnabled(enabled);
    _wsplitter->update();

    QList<int> mapIdxList = _imgMap.keys();
    for (int i=0; i != mapIdxList.size(); ++i)
    {
        _imgMap.value(mapIdxList.at(i))->setEnabled(enabled);
    }

    if ((enabled) && (_mode == "Blend"))
    {
        _blendSlider->setVisible(true);
    } else
    {
        _blendSlider->setVisible(false);
    }

    _display->update();
}


bool
RLQTDISP_Composite::mousePressEventInternal(QMouseEvent* event)
{
    bool result = false;

    return result;
}


bool
RLQTDISP_Composite::mouseMoveEventInternal(QMouseEvent* event)
{
    // RLP_LOG_DEBUG(event->position())

    bool result = false;

    // RLP_LOG_DEBUG("")

    return result;
}


bool
RLQTDISP_Composite::mouseReleaseEventInternal(QMouseEvent* event)
{
    bool result = false;

    return false;
}


void
RLQTDISP_Composite::recalculateImageRects(DS_BufferPtr fbuf, qreal wx, qreal wy, qreal zoom)
{
    QVariantMap* bm = fbuf->getBufferMetadata();

    // RLP_LOG_DEBUG(fbufWidth << fbufHeight)

    int compositeIdx = bm->value("node.composite.idx").value<int>();

    if (!_imgMap.contains(compositeIdx))
    {
        // Cannot construct CompositeImageWindow widget here since
        // we're in the render thread. Emit a signal instead for slot
        // connection in the main thread
        emit imageWindowNeeded(bm);
        return;
    }

    _imgMap.value(compositeIdx)->setEnabled(_visEnabled);
    _imgMap.value(compositeIdx)->refresh(wx, wy, zoom, bm);

    for (int fi=0; fi != fbuf->numAuxBuffers(false /* includePrimary */); ++fi)
    {
        DS_BufferPtr afbuf = fbuf->getAuxBuffer(fi);
        recalculateImageRects(afbuf, wx, wy, zoom);
    }

}


void
RLQTDISP_Composite::resizeGLInternal(int width, int height)
{
    // RLP_LOG_DEBUG(width << height)

    _tools->setPos(width - 100, 10);

    _wsplitter->setPos((float)width * _wsplitter->posPercent(), 0);
    _blendSlider->setPos(width - 350, 10);
}



void
RLQTDISP_Composite::paintGLPostDraw(GUI_Painter* painter, const QVariantMap& metadata)
{
    // RLP_LOG_DEBUG("fade message paint gl post draw" << std::endl;
    
    qreal wx = metadata.value("window.width").value<qreal>();
    qreal wy = metadata.value("window.height").value<qreal>();

    // RLP_LOG_DEBUG("wx:" << wx << "wy:" << wy)

    // if ((wx != _windowWidth) || (wy != _windowHeight))
    // {
    _windowWidth = wx;
    _windowHeight = wy;

    DS_FrameBufferPtr fbuf = metadata.value("video.framebuffer").value<DS_FrameBufferPtr>();
    qreal zoom = metadata.value("display.zoom").value<qreal>();

    QVariantMap* mbufm = fbuf->getBufferMetadata();
    if (!mbufm->contains("node.composite.idx"))
    {
        hideTools();
    } else
    {
        _tools->setVisible(true);
        if (mbufm->value("composite_mode").toString() == "Wipe")
        {
            _wsplitter->setVisible(true);
        }
        recalculateImageRects(fbuf, wx, wy, zoom);
    }
}

