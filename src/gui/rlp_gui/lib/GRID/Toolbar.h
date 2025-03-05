//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef GUI_GRID_TOOLBAR_H
#define GUI_GRID_TOOLBAR_H

#include "RlpGui/GRID/Global.h"
#include "RlpGui/BASE/HLayout.h"
#include "RlpGui/WIDGET/IconButton.h"
#include "RlpGui/WIDGET/Label.h"



class GUI_GRID_API GUI_GRID_Toolbar : public GUI_ItemBase {
    Q_OBJECT


public:
    RLP_DEFINE_LOGGER

    GUI_GRID_Toolbar(GUI_ItemBase* parent);

    QRectF boundingRect() const;

    void paint(GUI_Painter* painter);

public slots:

    GUI_IconButton* colSelector() { return _colSelector; }
    GUI_Label* label() { return _label; }
    GUI_HLayout* layout() { return _layout; }

private:

    GUI_HLayout* _layout;

    GUI_Label* _label;
    GUI_IconButton* _colSelector;

};

#endif