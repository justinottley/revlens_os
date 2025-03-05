//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef CORE_GUI_LAYOUT_BASE_H
#define CORE_GUI_LAYOUT_BASE_H

#include "RlpGui/BASE/Global.h"
#include "RlpGui/BASE/ItemBase.h"

typedef QList<GUI_ItemBase*>::iterator LayoutItemIterator;

class GUI_BASE_API GUI_LayoutBase : public GUI_ItemBase {
    Q_OBJECT

signals:
    void itemAdded(GUI_ItemBase* item);

public:
    RLP_DEFINE_LOGGER

    GUI_LayoutBase(QQuickItem* parent);
    virtual ~GUI_LayoutBase();

    virtual void addItem(GUI_ItemBase* item, int offset=0) {}
    virtual void addSpacer(int size) {}
    virtual void addDivider(int size, int osize=25) {}

    virtual QList<GUI_ItemBase*> items();

public slots:

    virtual qreal itemWidth() { return 0; }
    virtual qreal itemHeight() { return 0; }

    virtual void onParentSizeChanged(qreal width, qreal height);

    virtual void clear() {}

protected:

    QMap<GUI_ItemBase*, int> _offsetMap;
};

#endif