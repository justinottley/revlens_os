//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef GUI_SVG_BUTTON_H
#define GUI_SVG_BUTTON_H


#include "RlpGui/PANE/Global.h"
#include "RlpGui/BASE/Style.h"
#include "RlpGui/BASE/ItemBase.h"
#include "RlpGui/BASE/ButtonBase.h"
#include "RlpGui/MENU/MenuPane.h"

#include "RlpGui/BASE/SvgIcon.h"

class GUI_MenuPane;

class GUI_PANE_API GUI_SvgButton : public GUI_ButtonBase {
    Q_OBJECT

signals:
    void hoverChanged(bool inHover);

public:
    RLP_DEFINE_LOGGER

    GUI_SvgButton(QString filename,
                  QQuickItem* parent,
                  int size=20,
                  int padding=0,
                  QColor bgCol=GUI_Style::IconBg
    );

    QRectF boundingRect() const;

    virtual void mousePressEvent(QMouseEvent* event);

    virtual void hoverEnterEvent(QHoverEvent* event);
    virtual void hoverLeaveEvent(QHoverEvent* event);

    virtual void paint(GUI_Painter* painter);

public slots:

    static GUI_SvgButton*
    new_GUI_SvgButton(QString filename, QQuickItem* parent, int size)
    {
        return new GUI_SvgButton(filename, parent, size);
    }

    void connectToMenu();

    GUI_SvgIcon* icon() { return _icon; }
    GUI_MenuPane* menu() { return _menu; }

    qreal size() const { return _size; }

    void setHover(bool doHover) { _icon->setHover(doHover); }
    bool inHover() { return _icon->inHover(); }

    void setBgColor(QColor col) { _bgCol = col; }

    void setOutlined(bool outlined);
    void setOverlayText(QString text);
    QString overlayText() { return _overlayText; }

    // API Compat with ImageButton
    void setText(QString text) { setProperty("text", text); }
    QString text() { return property("text").toString(); }

    void setOverlayTextPos(int x, int y);

    void setIconPos(int x, int y);
    void setMenuXPosOffset(int xpo) { _menuXPosOffset = xpo; }
    void setMenuYPosOffset(int ypo) { _menuYPosOffset = ypo; }

    void setSvgIcon(GUI_SvgIcon* icon);

    void setOutlinedBgColour(QColor col) { _icon->setOutlinedBgColour(col); }
    void setHoverColour(QColor col) { _icon->setHoverColour(col); }

    void onIconHoverChanged(bool inHover);
    void onMenuItemSelected(QVariantMap sigData);
    void toggleMenu(QVariantMap mdata=QVariantMap());

protected:

    void initIcon(GUI_SvgIcon* icon);

    GUI_SvgIcon* _icon;

    QString _overlayText;
    int _overlayPosX;
    int _overlayPosY;

    QColor _bgCol;
    int _size;

    GUI_MenuPane* _menu;

    int _menuXPosOffset;
    int _menuYPosOffset;
};


class GUI_SvgToggleButton : public GUI_SvgButton {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    GUI_SvgToggleButton(
        QString filenameOn,
        QString filenameOff,
        QQuickItem* parent,
        int size=20,
        int padding=0,
        QColor bgCol=GUI_Style::IconBg
    );

    void setToggled(bool toggled);

public slots:

    static GUI_SvgToggleButton*
    new_GUI_SvgToggleButton(QString filenameOn, QString filenameOff, QQuickItem* parent, int size)
    {
        return new GUI_SvgToggleButton(filenameOn, filenameOff, parent, size);
    }

protected:
    GUI_SvgIcon* _onIcon;

};

#endif