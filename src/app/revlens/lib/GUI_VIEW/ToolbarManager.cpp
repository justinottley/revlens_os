
#include "RlpRevlens/GUI_VIEW/ToolbarManager.h"

RLP_SETUP_LOGGER(revlens, GUI_VIEW, ToolbarManager)

GUI_VIEW_ToolbarManager* GUI_VIEW_ToolbarManager::_instance = nullptr;

GUI_VIEW_ToolbarManager::GUI_VIEW_ToolbarManager()
{
    _instance = this;
}


void
GUI_VIEW_ToolbarManager::registerViewerToolbar(QVariantMap toolbarInfo)
{
    RLP_LOG_DEBUG(toolbarInfo)

    QString name = toolbarInfo.value("name").toString();

    _toolbarInfoMap.insert(name, toolbarInfo);
}