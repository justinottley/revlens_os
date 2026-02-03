
#include "RlpExtrevlens/RLOCIO/Plugin.h"
#include "RlpExtrevlens/RLOCIO/PaintEngine.h"

#include "RlpRevlens/CNTRL/MainController.h"
#include "RlpRevlens/CNTRL/PluginManager.h"

RLP_SETUP_EXT_LOGGER(RLOCIO, Plugin)

RLOCIO_Plugin::RLOCIO_Plugin():
    DS_EventPlugin("OpenColorIO")
{
    /*
    connect(
        CNTRL_MainController::instance(),
        SIGNAL(toolCreated(QVariantMap)),
        this,
        SLOT(onToolCreated(QVariantMap))
    );
    */
   /*
   RLP_LOG_DEBUG(CNTRL_MainController::instance());
   CNTRL_MainController* cntrl = CNTRL_MainController::instance();
   connect(
    cntrl,
    &CNTRL_MainController::toolCreated,
    this,
    &RLOCIO_Plugin::onToolCreated
   );
   */
    /*
        &CNTRL_MainController::toolCreated,
        this,
        &RLOCIO_Plugin::onToolCreated
    );
    */

   CNTRL_MainController::instance()->getPluginManager()->registerPaintEngine(
        "OCIO",
        &RLOCIO_PaintEngine::create
    );
}


void
RLOCIO_Plugin::onToolCreated(QVariantMap toolInfo)
{
    QString toolName = toolInfo.value("name").toString();
    if (toolName == "Viewer")
    {
        RLP_LOG_DEBUG(toolInfo)

        GUI_ItemBase* viewer = toolInfo.value("tool").value<GUI_ItemBase*>();
        _otools = new RLOCIO_OverlayTools(viewer);
        _otools->setVisible(false);
    }
}