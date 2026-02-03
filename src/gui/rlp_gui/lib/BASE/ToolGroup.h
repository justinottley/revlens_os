//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef CORE_GUI_TOOLGROUP_H
#define CORE_GUI_TOOLGROUP_H

#include "RlpGui/BASE/Global.h"
#include "RlpGui/BASE/ItemBase.h"
#include "RlpGui/BASE/LayoutBase.h"


class GUI_BASE_API GUI_ToolGroup : public GUI_ItemBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    enum ToolGroupOrientation {
        TG_HORIZONTAL,
        TG_VERTICAL
    };


    enum ToolGroupAlignment {
        TG_ALIGN_CENTER,
        TG_ALIGN_NONE
    };


    GUI_ToolGroup(
        GUI_ItemBase* parent,
        ToolGroupOrientation tgOrientation,
        ToolGroupAlignment alignment=TG_ALIGN_CENTER,
        int size=30);

    ~GUI_ToolGroup();

    QRectF boundingRect() const;

    void paint(GUI_Painter* painter);


public slots:

    void onParentSizeChanged(qreal nwidth, qreal nheight);

    void addItem(GUI_ItemBase* item, int offset=0);
    void addSpacer(int size) { _layout->addSpacer(size); }
    void addDivider(int size, int height=25, int voffset=5) { _layout->addDivider(size, height, voffset); }

    int itemWidth() { return _layout->itemWidth(); }

    GUI_LayoutBase* layout() { return _layout; }

protected:

    ToolGroupOrientation _orientation;
    ToolGroupAlignment _alignment;
    GUI_LayoutBase* _layout;
};


#endif