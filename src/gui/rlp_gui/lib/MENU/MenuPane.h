//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef GUI_MENU_MENUPANE_H
#define GUI_MENU_MENUPANE_H

#include "RlpGui/MENU/Global.h"

#include "RlpGui/BASE/ItemBase.h"
#include "RlpGui/BASE/MenuPaneBase.h"
#include "RlpGui/BASE/ScrollArea.h"

#include "RlpGui/MENU/MenuItem.h"

class GUI_MenuPane;

class GUI_MENU_API GUI_MenuPaneScrollButton : public GUI_ItemBase {
    Q_OBJECT

signals:
    void buttonPressed(int direction);

public:
    RLP_DEFINE_LOGGER

    static const int HEIGHT;
    static const int INC;

    enum ScrollDirection {
        MENU_SCROLL_DOWN,
        MENU_SCROLL_UP
    };

    GUI_MenuPaneScrollButton(GUI_MenuPane* mpane, ScrollDirection direction);

    void mousePressEvent(QMouseEvent* event);
    void hoverEnterEvent(QHoverEvent* event);
    void hoverLeaveEvent(QHoverEvent* event);

    void onParentSizeChanged(qreal width, qreal height);

    QRectF boundingRect() const;
    void paint(GUI_Painter* painter);

private:
    GUI_MenuPane* _mpane;
    ScrollDirection _direction;
    GUI_SvgIcon* _icon;
    QColor _bgCol;

};


class GUI_MENU_API GUI_MenuPane : public GUI_MenuPaneBase {
    Q_OBJECT

signals:
    void menuItemSelected(QVariantMap info);
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

    void updateDimensions();

    void paint(GUI_Painter* painter);

public slots:

    static GUI_MenuPane* new_GUI_MenuPane(GUI_ItemBase* parent)
    {
        return new GUI_MenuPane(parent);
    }

    void clear();
    void resetItemPositions();
    void updateScrolling();

    GUI_MenuItem* addItem(QString entry, QVariantMap data=QVariantMap(), bool checkable=false);
    void addWidgetItem(GUI_ItemBase* item) { addItem(item); } // for python

    void addSeparator();

    void setWidthPolicy(WidthPolicy wp) { _widthPolicy = wp; }
    void setPos(qreal x, qreal y);
    void setFilterString(QString filterStr);
    void setExclusiveSelect(bool es) { _exclusiveSelect = es; }

    void setBgColour(QColor col) { _bgCol = col; }

    bool isFocused() { return _focusedItems.size() != 0; }

    int itemCount() { return _items.size(); }
    GUI_MenuItem* item(QString name);

    GUI_ItemBase* itemWrapper() { return _itemWrapper; }

    int scrollHOffset();

    void onMenuItemSelected(QVariantMap menuData);
    void onMenuItemMenuShown(QString name);
    void onMenuItemHoverEntered(QString menuName);
    void onMenuItemHoverExited(QString menuName);
    void onScrollPressed(int direction);
    void onParentSizeChanged(qreal width, qreal height);


private:
    GUI_ScrollArea* _itemWrapper;

    QColor _bgCol;


    GUI_MenuPaneScrollButton* _scrollTop;
    GUI_MenuPaneScrollButton* _scrollBottom;
    bool _scrollButtonsVisible;

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