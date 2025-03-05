//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef GUI_MENU_BUTTON_H
#define GUI_MENU_BUTTON_H

#include "RlpGui/BASE/ItemBase.h"
#include "RlpGui/MENU/Global.h"
#include "RlpGui/MENU/MenuPane.h"

class GUI_MENU_API GUI_MenuButton : public GUI_ItemBase {
    Q_OBJECT

signals:
    void menuShown(QString text);
    void menuHidden(QString text);

public:
    RLP_DEFINE_LOGGER

    GUI_MenuButton(GUI_ItemBase* parent);

    void mousePressEvent(QMouseEvent* event);
    void hoverEnterEvent(QHoverEvent* event);
    void hoverLeaveEvent(QHoverEvent* event);

    QRectF boundingRect() const;
    void paint(GUI_Painter* painter);

public slots: // Compatibility API for Qt MenuBar widget

    GUI_MenuItem* addMenu(QString itemName, QVariantMap itemData=QVariantMap());
    GUI_MenuItem* addAction(QString itemName);
    void addSeparator();


public slots:

    GUI_MenuPane* menu() { return _menu; }

    QString text() { return _text; }

    void setLeftImage(QPixmap* image) { _leftImage = image; }
    void setRightImage(QPixmap* image) { _rightImage = image; }

    void setPos(qreal xpos, qreal ypos);
    void setText(QString text);
    void setBgColor(QColor col);

    void onMenuItemSelected(QVariantMap item);
    // void onSceneLayoutChanged();

    void setShowMenuOnHover(bool doMenuHover) { _showMenuOnHover = doMenuHover; }

private:
    
    QPixmap* _leftImage;
    QPixmap* _rightImage;
    
    QString _text;

    GUI_MenuPane* _menu;

    bool _showMenuOnHover;
    
    bool _focused;

    QColor _bgCol;
};



#endif