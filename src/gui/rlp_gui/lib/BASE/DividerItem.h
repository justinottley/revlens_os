//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef GUI_BASE_DIVIDER_ITEM_H
#define GUI_BASE_DIVIDER_ITEM_H


#include "RlpGui/BASE/Global.h"
#include "RlpGui/BASE/ItemBase.h"

class GUI_BASE_API GUI_DividerItem : public GUI_ItemBase {
    Q_OBJECT

public:

    GUI_DividerItem(qreal width, qreal height, GUI_ItemBase* parent);

    virtual void paint(GUI_Painter* painter);


};



#endif