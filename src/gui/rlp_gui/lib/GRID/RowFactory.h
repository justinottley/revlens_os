
#ifndef GUI_GRID_ROW_FACTORY_H
#define GUI_GRID_ROW_FACTORY_H

#include "RlpGui/GRID/Global.h"
#include "RlpGui/GRID/Row.h"
#include "RlpGui/GRID/RowWidgets.h"

class GUI_GRID_API GUI_GRID_RowTypePlugin : public QObject {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    GUI_GRID_RowTypePlugin(QString name);

    virtual GUI_GRID_Row* makeRow(GUI_ItemBase* parent, GUI_GRID_View* view, int rowIdx);

public slots:
    QString name() { return _name; }


private:
    QString _name;

};


class GUI_GRID_API GUI_GRID_RowFactory : public QObject {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    GUI_GRID_RowFactory();

    static GUI_GRID_RowFactory* instance();

    GUI_GRID_Row* makeRow(
        QVariantMap rowTypeInfo,
        GUI_ItemBase* parent,
        GUI_GRID_View* view,
        int rowIdx
    );

    void initRowWidget(
        GUI_GRID_Row* row,
        QString widgetName,
        QVariantMap widgetInfo
    );


public slots:

    static GUI_GRID_RowFactory*
    new_GUI_GRID_RowFactory()
    {
        return new GUI_GRID_RowFactory();
    }

    void registerRowTypePlugin(GUI_GRID_RowTypePlugin* plugin);
    void registerRowWidgetPlugin(QString widgetName, GUI_GRID_RowWidgetPlugin* plugin);


private:
    static GUI_GRID_RowFactory* _instance;

    QMap<QString, GUI_GRID_RowTypePlugin*> _rowTypeMap;
    QMap<QString, GUI_GRID_RowWidgetPlugin*> _rowWidgetMap;

};

#endif
