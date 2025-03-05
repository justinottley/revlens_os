
#include "RlpGui/BASE/ToolInfo.h"

// #include "RlpGui/BASE/QWidgetItem.h"

RLP_SETUP_LOGGER(gui, GUI, ToolInfo)

GUI_ToolInfo::GUI_ToolInfo(QVariantMap metadata):
    _data(metadata)
{
    if (metadata.contains("widget")) {

        RLP_LOG_INFO("creating widget wrapper for tool")
        QWidget* widget = metadata.value("widget").value<QWidget*>();
    }
}