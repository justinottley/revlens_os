//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef CORE_GUI_HLAYOUT_H
#define CORE_GUI_HLAYOUT_H

#include "RlpGui/BASE/Global.h"
#include "RlpGui/BASE/ItemBase.h"
#include "RlpGui/BASE/LayoutBase.h"
#include "RlpGui/BASE/SpacerItem.h"


class GUI_BASE_API GUI_HLayout : public GUI_LayoutBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    enum InsertPosition {
        POS_START,
        POS_END
    };

    GUI_HLayout(QQuickItem* parent, int hspacing=0);

    QRectF boundingRect() const;

    void paint(GUI_Painter* painter);

    QList<GUI_ItemBase*> items() { return _items; }

    void addItemAt(GUI_ItemBase* item, int voffset=0, InsertPosition pos=POS_END);

public slots:

    static GUI_HLayout* new_GUI_HLayout(QQuickItem* parent) { return new GUI_HLayout(parent); }

    void onItemSizeChanged();

    void addItem(GUI_ItemBase* item, int voffset=0);
    void addItemAtStart(GUI_ItemBase* item, int voffset=0);

    void addSpacer(int size);
    void addDivider(int size, int height=25, int voffset=0);

    qreal itemWidth() { return _itemWidth; }
    qreal itemHeight() { return _itemHeight; }

    void setSpacing(qreal spacing) { _hspacing = spacing; }

    void setBgColour(QColor col) { _bgcolour = col; }

    void setHeightOffset(int hoff) { _hoff = hoff; }
    void setWidthOffset(int woff) { _woff = woff; }

    QVariantList itemList(); // easier pybinding than items()

    void clear();

private:

    qreal _itemWidth; // width of all items so far
    qreal _itemHeight; // height item with max height
    int _hspacing;
    int _vspacing;
    int _hoff;
    int _woff;

    QList<GUI_ItemBase*> _items;

    QColor _bgcolour;

};



#endif