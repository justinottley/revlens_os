//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef CORE_GUI_FLOW_LAYOUT_H
#define CORE_GUI_FLOW_LAYOUT_H

#include "RlpCore/LOG/Logging.h"

#include "RlpGui/BASE/Global.h"
#include "RlpGui/BASE/ItemBase.h"
#include "RlpGui/BASE/LayoutBase.h"
#include "RlpGui/BASE/ScrollArea.h"

class GUI_BASE_API GUI_FlowLayout : public GUI_LayoutBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    GUI_FlowLayout(GUI_ItemBase* parent, int spacing=10);

    QRectF boundingRect() const;

    void paint(GUI_Painter* painter);

public slots:

    static GUI_FlowLayout*
    new_GUI_FlowLayout(GUI_ItemBase* parent)
    {
        return new GUI_FlowLayout(parent);
    }


    void addItem(GUI_ItemBase* item);

    void updateItems(); // reposition items in the flow layout

    void setSpacing(qreal spacing);
    void setOutlined(bool outlined);
    void onParentSizeChanged(qreal nwidth, qreal nheight);

    QList<GUI_ItemBase*> items() { return _items; }
    GUI_ScrollArea* scrollArea() { return _scrollArea; }
    void clear();

private:

    qreal _spacing;

    GUI_ScrollArea* _scrollArea;
    QList<GUI_ItemBase*> _items;

};


class GUI_BASE_API GUI_FlowLayoutDecorator : public QObject {
    Q_OBJECT

public slots:
    GUI_FlowLayout*
    new_GUI_FlowLayout(GUI_ItemBase* parent, int hspacing=0)
    {
        return new GUI_FlowLayout(parent, hspacing);
    }
};

#endif

