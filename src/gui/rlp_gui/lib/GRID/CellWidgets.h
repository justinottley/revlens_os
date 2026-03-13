

#ifndef GUI_GRID_CELL_WIDGETS_H
#define GUI_GRID_CELL_WIDGETS_H

#include "RlpGui/GRID/Global.h"

#include "RlpGui/MENU/MenuPane.h"
#include "RlpGui/PANE/SvgButton.h"

class GUI_GRID_Cell;


class GUI_GRID_API GUI_GRID_CellWidget : public GUI_ItemBase {
    Q_OBJECT

public:
    GUI_GRID_CellWidget(GUI_GRID_Cell* cell);

    virtual void onParentSizeChanged(qreal width, qreal height);

    GUI_GRID_Cell* cell() { return _cell; }

protected:
    GUI_GRID_Cell* _cell;
};


class GUI_GRID_API GUI_GRID_CellChoiceWidget : public GUI_GRID_CellWidget {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    GUI_GRID_CellChoiceWidget(GUI_GRID_Cell* cell, QVariantMap widgetInfo);

    virtual void initMenu() {}
    void onParentSizeChanged(qreal width, qreal height);

public slots:
    GUI_SvgButton* menuButton() { return _menuButton; }
    GUI_MenuItem* menuWidget() { return _menuWidget; }

    void onMenuButtonPressed(QVariantMap md);
    void onMenuItemSelected(QVariantMap md);
    void onMenuWidgetSelectionChanged(QString selMode, QVariantList selInfo);

private:
    void buildWidgets();


protected:
    QVariantMap _widgetInfo;

    GUI_SvgButton* _menuButton;
    GUI_MenuItem* _menuWidget;

    bool _built;

};


class GUI_GRID_API GUI_GRID_CellHoverButtonWidget : public GUI_GRID_CellWidget {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    GUI_GRID_CellHoverButtonWidget(GUI_GRID_Cell* cell, QVariantMap initInfo);

    void cellHoverEnterEvent(QHoverEvent* event);
    void cellHoverLeaveEvent(QHoverEvent* event);

    void onParentSizeChanged(int width, int height);

public slots:
    GUI_SvgButton* button() { return _button; }


protected:
    GUI_SvgButton* _button;
    QVariantMap _initInfo;

};


#endif
