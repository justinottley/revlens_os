//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef CORE_GUI_TOOLGROUP_H
#define CORE_GUI_TOOLGROUP_H

#include "RlpGui/BASE/Global.h"
#include "RlpGui/BASE/ItemBase.h"
#include "RlpGui/BASE/LayoutBase.h"

enum ToolGroupOrientation {
    TG_HORIZONTAL,
    TG_VERTICAL
};


class GUI_BASE_API GUI_ToolGroup : public GUI_ItemBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    GUI_ToolGroup(
        ToolGroupOrientation tgOrientation,
        GUI_ItemBase* parent,
        int size=30);

    ~GUI_ToolGroup();

    QRectF boundingRect() const;

    void paint(GUI_Painter* painter);


public slots:

    void onParentSizeChanged(qreal nwidth, qreal nheight);

    void addItem(GUI_ItemBase* item, int offset=0);
    void addSpacer(int size) { _layout->addSpacer(size); }
    void addDivider(int size, int height=25) { _layout->addDivider(size, height); }

    int itemWidth() { return _layout->itemWidth(); }

    GUI_LayoutBase* layout() { return _layout; }

protected:

    ToolGroupOrientation _orientation;
    GUI_LayoutBase* _layout;
};


#endif