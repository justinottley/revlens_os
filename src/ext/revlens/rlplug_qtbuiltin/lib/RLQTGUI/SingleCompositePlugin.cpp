
#include "RlpExtrevlens/RLQTGUI/SingleCompositePlugin.h"

RLP_SETUP_LOGGER(extrevlens, RLQTGUI, SingleCompositePlugin)


RLQTGUI_SingleCompositePlugin::RLQTGUI_SingleCompositePlugin():
    DISP_CompositePlugin("Single")
{
}


void
RLQTGUI_SingleCompositePlugin::layout(DS_BufferPtr destFrame, QVariantMap* displayMetadata)
{
    RLP_LOG_DEBUG("")

    QVariantMap* mbufm = destFrame->getBufferMetadata();
    mbufm->insert("alpha", 1.0);
    mbufm->insert("layout_scale.x", 1.0);
    mbufm->insert("layout_scale.y", 1.0);
    mbufm->insert("layout_pan.x", 0.0);
    mbufm->insert("layout_pan.y", 0.0);
}
