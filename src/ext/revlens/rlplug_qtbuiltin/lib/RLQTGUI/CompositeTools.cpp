
#include "RlpExtrevlens/RLQTGUI/CompositeTools.h"

#include "RlpRevlens/CNTRL/MainController.h"
#include "RlpRevlens/CNTRL/MediaManager.h"
#include "RlpRevlens/CNTRL/Video.h"


RLP_SETUP_LOGGER(extrevlens, RLQTGUI, CompositeTools)

RLP_SETUP_LOGGER(extrevlens, RLQTGUI, ViewerIconButton)
RLP_SETUP_LOGGER(extrevlens, RLQTGUI, CompositeModeButton)

RLQTGUI_ViewerIconButton::RLQTGUI_ViewerIconButton(GUI_ItemBase* parent, QString iconPath, int height):
    GUI_IconButton(iconPath, parent, height)
{
    connect(
        this,
        &RLQTGUI_CompositeModeButton::menuShown,
        this,
        &RLQTGUI_CompositeModeButton::onMenuShown
    );
}


void
RLQTGUI_ViewerIconButton::onMenuShown(QVariantMap md)
{
    // RLP_LOG_DEBUG(md)

    QPointF mbpos = mapToScene(pos());
    menu()->setPos(mbpos.x() -  pos().x(), mbpos.y() + 20);
}


RLQTGUI_CompositeModeButton::RLQTGUI_CompositeModeButton(GUI_ItemBase* parent):
    RLQTGUI_ViewerIconButton(parent, ":misc/layer.png", 15)
{
    _textYOffset = 0;

    setOutlined(true);

    CompositePluginMap* cpm = CNTRL_MainController::instance()->getVideoManager()->getCompositePlugins();
    CompositePluginMap::iterator it;

    for (it = cpm->begin(); it != cpm->end(); ++it)
    {
        addAction(it.key());
    }

    setText("Single");
}


void
RLQTGUI_CompositeModeButton::onModeMenuItemSelected(QVariantMap itemInfo)
{
    QString itemName = itemInfo.value("text").toString();

    setText(itemName);

    RLP_LOG_ERROR("SKIPPING")
    return;


    // DS_FrameBufferPtr fbuf = _display->frameBuffer();
    // QVariantMap* bm = fbuf->getBufferMetadata();

    // if (!bm->contains("node.composite.idx"))
    // {
    //     return;
    // }

    // QString nodeUuidStr = bm->value("graph.uuid").toString();

    // RLP_LOG_DEBUG("Switching Composite Mode to:" << itemName << "for node:" << nodeUuidStr)

    CNTRL_MainController* controller = CNTRL_MainController::instance();
    qlonglong fnum = controller->currentFrameNum();
    DS_NodePtr node = controller->session()->getNodeByFrame(fnum);
    if (!node)
    {
        RLP_LOG_ERROR("Valid Node not not available at frame" << fnum)
        return;
    }

    // DS_NodePtr node = controller->session()->getNodeByUuid(nodeUuidStr);
    node->setProperty("display.composite_mode", itemName);


    // need to eject everything from the lookahead to recache the metadata
    // into the framebuffers
    CNTRL_Video* cv = controller->getVideoManager();
    cv->clearAll();
    cv->updateToFrame(fnum, true);

    QVariantMap ninfo;
    ninfo.insert("composite_mode", itemName);

    controller->emitNoticeEvent("composite_mode_changed", ninfo);

    // emit compositeModeChanged(itemName);
}

// --------------------

RLQTGUI_CompositeTools::RLQTGUI_CompositeTools(GUI_ItemBase* parent):
    GUI_ItemBase(parent)
{
    _modeButton = new GUI_IconButton(":misc/layer.png", this, 15);

    CompositePluginMap* cpm = CNTRL_MainController::instance()->getVideoManager()->getCompositePlugins();
    CompositePluginMap::iterator it;
    for (it = cpm->begin(); it != cpm->end(); ++it)
    {
        _modeButton->addAction(it.key());
    }

    CNTRL_MainController* controller = CNTRL_MainController::instance();
    qlonglong fnum = controller->currentFrameNum();
    DS_NodePtr node = controller->session()->getNodeByFrame(fnum);
    if (!node)
    {
        RLP_LOG_ERROR("Valid Node not not available at frame" << fnum)
        return;
    }

    // DS_NodePtr node = controller->session()->getNodeByUuid(nodeUuidStr);
    if (node->hasProperty("display.composite_mode"))
    {
        QString currMode = node->getPropertiesPtr()->value("display.composite_mode").toString();
        _modeButton->setText(currMode);
    }
}

