
#include "RlpGui/GRID/RowFactory.h"


GUI_GRID_RowFactory* GUI_GRID_RowFactory::_instance = nullptr;

RLP_SETUP_LOGGER(gui, GUI_GRID, RowTypePlugin)
RLP_SETUP_LOGGER(gui, GUI_GRID, RowFactory)

GUI_GRID_RowTypePlugin::GUI_GRID_RowTypePlugin(QString name):
    _name(name)
{
}


GUI_GRID_Row*
GUI_GRID_RowTypePlugin::makeRow(GUI_ItemBase* parent, GUI_GRID_View* view, int rowIdx)
{
    return new GUI_GRID_Row(parent, view, rowIdx);
}


GUI_GRID_RowFactory::GUI_GRID_RowFactory()
{
    if (GUI_GRID_RowFactory::_instance == nullptr)
    {
        RLP_LOG_INFO("Initializing instance")

        GUI_GRID_RowFactory::_instance = this;


        GUI_GRID_RowTypePlugin* baseRowTypePlugin = new GUI_GRID_RowTypePlugin("default");
        registerRowTypePlugin(baseRowTypePlugin);
    }
}


GUI_GRID_RowFactory*
GUI_GRID_RowFactory::instance()
{
    GUI_GRID_RowFactory* result = GUI_GRID_RowFactory::_instance;
    if (result == nullptr)
    {
        result = new GUI_GRID_RowFactory();
    }

    return result;
}


void
GUI_GRID_RowFactory::registerRowTypePlugin(GUI_GRID_RowTypePlugin* plugin)
{
    RLP_LOG_DEBUG(plugin->name())

    _rowTypeMap.insert(plugin->name(), plugin);
}


void
GUI_GRID_RowFactory::registerRowWidgetPlugin(QString widgetName, GUI_GRID_RowWidgetPlugin* plugin)
{
    RLP_LOG_DEBUG(widgetName << plugin)

    _rowWidgetMap.insert(widgetName, plugin);
}


GUI_GRID_Row*
GUI_GRID_RowFactory::makeRow(
    QVariantMap rowTypeInfo,
    GUI_ItemBase* parent,
    GUI_GRID_View* view,
    int rowIdx
)
{
    QString pluginName = "default";
    if (rowTypeInfo.contains("name"))
    {
        pluginName = rowTypeInfo.value("name").toString();
    }

    GUI_GRID_RowTypePlugin* plug = _rowTypeMap.value(pluginName);
    return plug->makeRow(parent, view, rowIdx);
}


void
GUI_GRID_RowFactory::initRowWidget(
    GUI_GRID_Row* row,
    QString widgetName,
    QVariantMap widgetInfo
)
{
    if (_rowWidgetMap.contains(widgetName))
    {
        GUI_GRID_RowWidgetPlugin* plug = _rowWidgetMap.value(widgetName);
        plug->initRowWidget(row, widgetInfo);
    }
}