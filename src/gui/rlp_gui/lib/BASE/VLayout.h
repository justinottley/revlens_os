//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef GUI_BASE_VLAYOUT_H
#define GUI_BASE_VLAYOUT_H

#include "RlpGui/BASE/Global.h"
#include "RlpGui/BASE/ItemBase.h"
#include "RlpGui/BASE/LayoutBase.h"
#include "RlpGui/BASE/SpacerItem.h"

class GUI_BASE_API GUI_VLayout : public GUI_LayoutBase {
    Q_OBJECT


public:
    RLP_DEFINE_LOGGER

    GUI_VLayout(QQuickItem* parent);

    QRectF boundingRect() const;

    void paint(GUI_Painter* painter);

public slots:

    static GUI_VLayout* new_GUI_VLayout(QQuickItem* parent) { return new GUI_VLayout(parent); }

    qreal itemHeight() { return _itemHeight; }
    qreal itemWidth() { return _itemWidth; }

    void onItemSizeChanged();
    void onParentSizeChanged(qreal width, qreal height);
    void addItem(GUI_ItemBase* item, int hoffset=0);
    void addSpacer(int size);

    void setHSpacing(int spacing) { _hspacing = spacing; }
    void setVSpacing(int vspacing) { _vspacing = vspacing; }

    void clear();

    QList<GUI_ItemBase*> items() { return _items; }

    bool removeItem(GUI_ItemBase* item);

private:

    int _itemWidth;
    int _itemHeight; // height of all items so far
    int _hspacing;
    int _vspacing;

    QList<GUI_ItemBase*> _items;

};


#endif