//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef CORE_GUI_MENUBAR_H
#define CORE_GUI_MENUBAR_H

#include "RlpGui/MENU/Global.h"

#include "RlpGui/BASE/ItemBase.h"
#include "RlpGui/BASE/ButtonBase.h"
#include "RlpGui/BASE/HLayout.h"

#include "RlpGui/MENU/MenuButton.h"

typedef QMap<QString, GUI_MenuButton*>::iterator MenuIterator;

class GUI_MENU_API GUI_MenuBar : public GUI_ItemBase {
    Q_OBJECT

signals:

    void menuShown(QString menuName, int visCount);
    void menuHidden(QString menuName, int visCount);


public:
    RLP_DEFINE_LOGGER

    GUI_MenuBar(GUI_ItemBase* parent);

    QRectF boundingRect() const;

    void onParentSizeChanged(qreal width, qreal height);
    void paint(GUI_Painter* painter);


public slots:

    static GUI_MenuBar* new_GUI_MenuBar(GUI_ItemBase* parent)
    {
        return new GUI_MenuBar(parent);
    }

    GUI_MenuButton* addMenu(QString menuName);
    void addMenuButton(GUI_ButtonBase* button);

    GUI_MenuButton* addRightMenu(QString menuName);
    void addRightItem(GUI_ItemBase* item);
    void addSpacer(int size) { _layout->addSpacer(size); }


    GUI_MenuButton* menu(QString menuName) { return _menus.value(menuName); }
    GUI_HLayout* layout() { return _layout; }

    void setMenuVisibility(bool isVisible);
    void onMenuShown(QString menuName);

    void onMenuHidden(QString menuName);
    void onMenuPanesHidden();
    void onMenuButtonShown(QVariantMap md);
    void onMenuButtonHidden(QVariantMap md);

    void clear();

private:

    GUI_HLayout* _layout;
    QMap<QString, GUI_MenuButton*> _menus;
    QList<GUI_ItemBase*> _rightItems;

    bool _menuVisible;

    int _visCount;
};

#endif