
#include "RlpExtrevlens/RLQTGUI/SingleCompositePlugin.h"

RLP_SETUP_LOGGER(extrevlens, RLQTGUI, SingleCompositePlugin)


RLQTGUI_SingleCompositePlugin::RLQTGUI_SingleCompositePlugin():
    DISP_CompositePlugin("Single")
{
}


void
RLQTGUI_SingleCompositePlugin::readVideo(DS_Node* nodeIn, DS_TimePtr timeInfo, DS_BufferPtr destFrame)
{
    // RLP_LOG_DEBUG("!!!")

    QVariantMap* mbufm = destFrame->getBufferMetadata();
    mbufm->insert("composite_mode", _name);


    int selectIdx = 0;
    if (nodeIn->getPropertiesPtr()->contains("display.composite.single_select"))
    {
        selectIdx = nodeIn->getPropertiesPtr()->value("display.composite.single_select").toInt();
    }

    DS_NodePtr node = nodeIn->input(selectIdx);
    node->readVideo(timeInfo, destFrame, /* postUpdate = */true);

}