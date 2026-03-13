
#include "RlpExtrevlens/RLOCIO/Init.h"
#include "RlpExtrevlens/RLOCIO/PaintEngine.h"
#include "RlpExtrevlens/RLOCIO/OverlayTools.h"

#include "RlpRevlens/CNTRL/MainController.h"
#include "RlpRevlens/CNTRL/PluginManager.h"


RLP_SETUP_LOGGER(ext, RLOCIO, Init)


RLOCIO_Init* RLOCIO_Init::_INSTANCE = nullptr;

void
RLOCIO_Init::init()
{
    if (_INSTANCE == nullptr)
    {
        _INSTANCE = new RLOCIO_Init();
        _INSTANCE->initTools();

        RLP_LOG_DEBUG("OK") 

    } else
    {

        RLP_LOG_WARN("Already Initialized")
    }
}


void
RLOCIO_Init::initTools()
{
    CNTRL_MainController::instance()->getPluginManager()->registerPaintEngine(
        "OCIO",
        &RLOCIO_PaintEngine::create
    );

    connect(
        CNTRL_MainController::instance(),
        &CNTRL_MainController::toolCreated,
        this,
        &RLOCIO_Init::onToolCreated
    );
}


void
RLOCIO_Init::onToolCreated(QVariantMap toolInfo)
{
    QString toolName = toolInfo.value("name").toString();
    if (toolName == "Viewer")
    {
        GUI_ItemBase* viewer = toolInfo.value("tool").value<GUI_ItemBase*>();
        _otools = new RLOCIO_OverlayTools(viewer);
        _otools->setVisible(false);
    }
}