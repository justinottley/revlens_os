
#ifndef GUI_GRID_ROW_WIDGETS_H
#define GUI_GRID_ROW_WIDGETS_H

#include "RlpGui/GRID/Global.h"


class GUI_GRID_Row;


class GUI_GRID_API GUI_GRID_RowWidget : public GUI_ItemBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    GUI_GRID_RowWidget(GUI_GRID_Row* row);

public slots:

    virtual void onParentSizeChanged(qreal width, qreal height);

protected:
    GUI_GRID_Row* _row;

};


// ----


class GUI_GRID_API GUI_GRID_RowWidgetPlugin : public QObject {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    GUI_GRID_RowWidgetPlugin(QString name);

    virtual void initRowWidget(GUI_GRID_Row* row, QVariantMap widgetInfo) {}

public slots:

    QString name() { return _name; }


protected:
    QString _name;

};

#endif