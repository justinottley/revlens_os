//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef GUI_MENU_MENUPANE_H
#define GUI_MENU_MENUPANE_H

#include "RlpGui/BASE/ItemBase.h"
#include "RlpGui/BASE/MenuPaneBase.h"
#include "RlpGui/MENU/Global.h"
#include "RlpGui/MENU/MenuItem.h"


class GUI_MENU_API GUI_MenuPane : public GUI_MenuPaneBase {
    Q_OBJECT

signals:
    void menuItemSelected(QVariantMap);
    void menuShown(QString name);


public:
    RLP_DEFINE_LOGGER
    
    enum WidthPolicy {
        AutoWidth,
        FixedWidth
    };

    GUI_MenuPane(GUI_ItemBase* parent = nullptr);
    ~GUI_MenuPane();

    void addItem(GUI_ItemBase* item);
    void setPos(QPointF pos);

public slots:

    static GUI_MenuPane* new_GUI_MenuPane(GUI_ItemBase* parent)
    {
        return new GUI_MenuPane(parent);
    }

    void clear();

    GUI_MenuItem* addItem(QString entry, QVariantMap data=QVariantMap(), bool checkable=false);
    void addSeparator();

    void setWidthPolicy(WidthPolicy wp) { _widthPolicy = wp; }
    void setPos(qreal x, qreal y);
    void setFilterString(QString filterStr);
    void setExclusiveSelect(bool es) { _exclusiveSelect = es; }

    bool isFocused() { return _focusedItems.size() != 0; }

    int itemCount() { return _items.size(); }
    GUI_MenuItem* item(QString name);

    void onParentSizeChanged(qreal width, qreal height);

public slots:

    void onMenuItemSelected(QVariantMap menuData);
    void onMenuItemMenuShown(QString name);
    void onMenuItemHoverEntered(QString menuName);
    void onMenuItemHoverExited(QString menuName);

public:

    QRectF boundingRect() const;

    void paint(GUI_Painter* painter);


private:

    WidthPolicy _widthPolicy;
    int _fixedWidth;
    int _autoWidth;

    QSet<QString> _focusedItems; // maintain a set of focused items so we know when we can autohide the menu

    qreal _itemHeight;

    QList<QQuickItem*> _items;
    QString _filterStr;
    bool _exclusiveSelect;

};


#endif