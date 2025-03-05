//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef CORE_GUI_ICON_BUTTON_H
#define CORE_GUI_ICON_BUTTON_H

#include "RlpGui/WIDGET/Global.h"
#include "RlpGui/BASE/Style.h"
#include "RlpGui/BASE/ButtonBase.h"
#include "RlpGui/BASE/SvgIcon.h"
#include "RlpGui/MENU/MenuPane.h"


class GUI_WIDGET_API GUI_IconButton : public GUI_ButtonBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    GUI_IconButton(QString filename, QQuickItem* parent, int size=DEFAULT_ICON_SIZE, int padding=4);

    virtual void mousePressEvent(QMouseEvent* event);
    virtual void hoverEnterEvent(QHoverEvent* event);
    virtual void hoverLeaveEvent(QHoverEvent* event);

    QRectF boundingRect() const;


    virtual void paint(GUI_Painter* painter);

public slots:

    static GUI_IconButton*
    new_GUI_IconButton(QString filename, QQuickItem* parent, int size=DEFAULT_ICON_SIZE, int padding=4)
    {
        return new GUI_IconButton(filename, parent, size, padding);
    }

    void onButtonPressed(QVariantMap metadata);
    void onMenuItemSelected(QVariantMap sigData);

    void setOutlined(bool outlined) { _drawOutline = outlined; }
    void setOutlinedBgColour(QColor col) { _outlineBgCol = col; }
    QColor outlineColour() { return _outlineBgCol; }
    void setHoverColour(QColor col) { _hoverCol = col; }

    void setSvgIconPath(QString filename, int size=-1, bool changeColour=true);
    void setSvgIconRight(bool doRight) { _svgIconRight = doRight; }
    void setSvgIconShowOnHover(bool ssh);
    void hideMenu();

    GUI_SvgIcon* svgIcon() { return _svgIcon; }
    void setSvgIcon(GUI_SvgIcon* icon) { _svgIcon = icon; }

    QImage imgIcon() { return _icon; }

    // API compat  with SvgButton
    GUI_IconButton* icon() { return this; }

    void setText(QString text);
    QString text() { return _text; }

    void setIconHOffset(int offset) { _iconHOffset = offset; }
    void setIconYOffset(int offset) { _iconYOffset = offset; }

    void setTextHOffset(int offset) { _textHOffset = offset; }
    void setTextYOffset(int offset) { _textYOffset = offset; }

    void setMenuXOffset(int offset) { _menuXOffset = offset; }
    void setMenuYOffset(int offset) { _menuYOffset = offset; }


    void setToggled(bool isToggled);
    bool toggled() { return _toggled; }

    GUI_MenuItem* addAction(QString itemName);


protected:

    QImage _icon;
    GUI_SvgIcon* _svgIcon;
    bool _svgIconRight;
    bool _svgIconShowOnHover;

    QString _text;

    QString _filename;

    int _padding;

    int _iconHOffset;
    int _iconYOffset;

    int _textHOffset;
    int _textYOffset;

    int _menuXOffset;
    int _menuYOffset;

    bool _drawOutline;
    QColor _outlineBgCol;

    QColor _hoverCol;

    bool _toggled;

};

/*

class GUI_WIDGET_API GUI_IconButtonDecorator : public QObject {
    Q_OBJECT

public slots:

    GUI_IconButton*
    new_GUI_IconButton(QString iconPath, GUI_ItemBase* parent, int size=15, int padding=4) {
        return new GUI_IconButton(iconPath, parent, size, padding);
    }
};
*/

#endif