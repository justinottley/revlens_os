//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef GUI_MENU_ITEM_H
#define GUI_MENU_ITEM_H

#include "RlpGui/MENU/Global.h"

#include "RlpGui/BASE/ItemBase.h"
#include "RlpGui/BASE/SvgIcon.h"

class GUI_MenuPane;

class GUI_MENU_API GUI_MenuItem : public GUI_ItemBase {
    Q_OBJECT
    QML_ELEMENT

signals:

    void triggered();
    void menuItemSelected(QVariantMap info);
    void menuItemTextChanged(QString text);
    void menuItemHoverEntered(QString text);
    void menuItemHoverExited(QString text);
    void menuShown(QString text);

public:
    RLP_DEFINE_LOGGER

    GUI_MenuItem(QString text, GUI_ItemBase* parent);

    void onParentSizeChanged(qreal width, qreal height);

    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void hoverEnterEvent(QHoverEvent* event);
    void hoverLeaveEvent(QHoverEvent* event);

    QRectF boundingRect() const;

    void paint(GUI_Painter* painter);

public slots: // Compatibility with Qt Widget

    static GUI_MenuItem* new_GUI_MenuItem(QString text, GUI_ItemBase* parent)
    {
        return new GUI_MenuItem(text, parent);
    }

    GUI_MenuItem* addMenu(QString itemName) { return addItem(itemName); }
    GUI_MenuItem* addAction(QString itemName) { return addItem(itemName); }

    void setShortcuts(QKeySequence::StandardKey key) {}
    void setStatusTip(QString statusTip) { _statusTip = statusTip; }
    void setToolTip(QString toolTip) { _toolTip = toolTip; }

    void selectItem(); // select programatically
    void selectItemWithValue(bool val) { _checked = !val; selectItem(); }

    void setText(QString text);
    QString text() { return _text; }

    void setData(QVariantMap data) { _data = data; }
    QVariant dataItem(QString key) { return _data.value(key); }
    QVariantMap data() { return _data; }

    void insertDataValue(QString key, QVariant val) { _data.insert(key, val); }
    void insertData(QVariantMap data) { _data.insert(data); }

    void setSelectable(bool selectable) { _selectable = selectable; }
    bool isSelectable() { return _selectable; }

    void setCheckable(bool checkable);
    bool isCheckable() { return _checkable; }

    void setChecked(bool checked);
    bool isChecked() { return _checked; }

    void setItemEnabled(bool enabled) { _enabled = enabled; }
    bool isItemEnabled() { return _enabled; }

    GUI_MenuPane* menu() { return _menu; }

    GUI_MenuItem* addItem(QString itemName, QVariantMap itemData=QVariantMap());

    void onSiblingMenuShown(QString itemName);
    void hideMenu();

    void setAutoHideMenu(bool doAutoHide) { _autoHideMenu = doAutoHide; }

private:

    QString _text;

    bool _focused;
    bool _pressed;
    bool _selectable;
    bool _enabled;
    bool _checkable;
    bool _checked;

    QString _statusTip;
    QString _toolTip;

    QVariantMap _data;

    GUI_MenuPane* _menu;
    bool _isMenu;
    bool _autoHideMenu;

    GUI_SvgIcon* _checkIcon;
    GUI_SvgIcon* _leftIcon;
    GUI_SvgIcon* _arrowIcon;
};

#endif
