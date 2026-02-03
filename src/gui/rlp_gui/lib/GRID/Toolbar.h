//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef GUI_GRID_TOOLBAR_H
#define GUI_GRID_TOOLBAR_H

#include "RlpGui/GRID/Global.h"
#include "RlpGui/BASE/HLayout.h"
#include "RlpGui/WIDGET/IconButton.h"
#include "RlpGui/WIDGET/Label.h"

class GUI_GRID_Toolbar;

class GUI_GRID_API GUI_GRID_ToolbarInit : public QObject {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    GUI_GRID_ToolbarInit();

    void init(GUI_GRID_Toolbar*);

public slots:

    static GUI_GRID_ToolbarInit*
    new_GUI_GRID_ToolbarInit()
    {
        return new GUI_GRID_ToolbarInit();
    }

    void setPyCallback(py::object callback) { _pycallback = callback; }

private:
    py::object _pycallback;
};


class GUI_GRID_API GUI_GRID_Toolbar : public GUI_ItemBase {
    Q_OBJECT


public:
    RLP_DEFINE_LOGGER

    GUI_GRID_Toolbar(GUI_ItemBase* parent, GUI_GRID_ToolbarInit* tbi);

    QRectF boundingRect() const;

    void paint(GUI_Painter* painter);

public slots:

    GUI_IconButton* colSelector() { return _colSelector; }
    GUI_HLayout* layout() { return _layout; }

private:

    GUI_HLayout* _layout;

    GUI_IconButton* _colSelector;

};

#endif