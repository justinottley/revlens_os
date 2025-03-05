//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef CORE_GUI_LABEL_H
#define CORE_GUI_LABEL_H

#include "RlpGui/WIDGET/Global.h"
#include "RlpGui/BASE/ItemBase.h"
#include "RlpGui/BASE/SvgIcon.h"

class GUI_WIDGET_API GUI_Label : public GUI_ItemBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    GUI_Label(QQuickItem* parent=nullptr, QString text="");
    
    QRectF boundingRect() const;

    void paint(GUI_Painter* painter);


public slots:

    static GUI_Label*
    new_GUI_Label(QQuickItem* parent, QString text)
    {
        return new GUI_Label(parent, text);
    }


    void setText(QString text);
    QString text() { return _text; }

    void setBgColor(QColor col);

    /*
    void setWidth(int width) { _width = width; }
    qreal width() { return _width; }

    void setHeight(int height) { _height = height; }
    qreal height() { return _height; }
    */

    void setLeftPadding(int pad) { _leftPad = pad; }
    void setYOffset(int off) { _yOffset = off; setHeight(20 + _yOffset); }

    void setDropShadow(bool doDropShadow) { _doDropShadow = doDropShadow; }
    void setPen(QPen pen) { _pen = pen; }

    QFont font() { return _font; }
    void setFont(QFont f);

    QImage leftImage() { return _leftImage; }
    void setLeftImagePath(QString imagePath, int width=0, int height=0);


    GUI_SvgIcon* leftSvgIcon() { return _leftSvgIcon; }
    void setLeftSvgIconPath(QString imagePath, int size, int padding=0);


private:
    
    
    QString _text;
    
    QImage _leftImage;
    GUI_SvgIcon* _leftSvgIcon;
    // QPixmap* _rightImage;
    
    // qreal _width;
    // qreal _height;

    int _leftPad;
    int _yOffset;

    QColor _bgCol;
    QPen _pen;
    QFont _font;

    bool _doDropShadow;
    int _dsXOffset;
    int _dsYOffset;
};


/*
class GUI_WIDGET_API GUI_LabelDecorator : public QObject {
    Q_OBJECT

public slots:

    GUI_Label*
    new_GUI_Label(GUI_ItemBase* parent, QString text)
    {
        return new GUI_Label(parent, text);
    }
};
*/

#endif