//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef GUI_BASE_SPACER_ITEM_H
#define GUI_BASE_SPACER_ITEM_H


#include "RlpGui/BASE/Global.h"
#include "RlpGui/BASE/ItemBase.h"

class GUI_BASE_API GUI_SpacerItem : public GUI_ItemBase {
    Q_OBJECT

public:

    GUI_SpacerItem(qreal width, qreal height, GUI_ItemBase* parent);

    virtual QRectF boundingRect() const;

    virtual void paint(GUI_Painter* painter) {}


};



#endif