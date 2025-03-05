//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef RLANN_GUI_BRUSH_SIZE_BUTTON_H
#define RLANN_GUI_BRUSH_SIZE_BUTTON_H

#include "RlpExtrevlens/RLANN_GUI/Global.h"

#include "RlpGui/BASE/ItemBase.h"
#include "RlpGui/PANE/SvgButton.h"


class EXTREVLENS_RLANN_GUI_API RLANN_GUI_BrushSizeMenuItem : public GUI_ItemBase {
    Q_OBJECT

signals:
    void valueChanged(QVariantMap val);
    void colourSelected(QVariantMap col);

public:
    RLP_DEFINE_LOGGER

    RLANN_GUI_BrushSizeMenuItem(GUI_MenuPane* parent);

    void setValue(qreal value);

    qreal width() { return _width; }
    qreal height() { return _height; }
    
    QRectF boundingRect() const;

    void paint(GUI_Painter* painter);

    void initColourButtons();


public slots:

    void onUpPressed(QVariantMap metadata);
    void onDownPressed(QVariantMap metadata);

    void onColourSelected(QColor col);


private:
    QString _text;
    qreal _value;

    qreal _width;
    qreal _height;
};


class EXTREVLENS_RLANN_GUI_API RLANN_GUI_BrushSizeButton : public GUI_SvgButton {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    RLANN_GUI_BrushSizeButton(GUI_ItemBase* parent);

    void paint(GUI_Painter* painter);


public slots:

    static RLANN_GUI_BrushSizeButton*
    new_RLANN_GUI_BrushSizeButton(GUI_ItemBase* parent)
    {
        return new RLANN_GUI_BrushSizeButton(parent);
    }


    void setValue(qreal value);
    void onButtonPressed(QVariantMap metadata);
    void onColourSelected(QVariantMap colInfo);

    RLANN_GUI_BrushSizeMenuItem* sizeItem() { return _sizeMenuItem; }


private:
    RLANN_GUI_BrushSizeMenuItem* _sizeMenuItem;

    QColor _col; // current color

};


class EXTREVLENS_RLANN_GUI_API RLANN_GUI_BrushSizeButtonDecorator : public QObject {
    Q_OBJECT

public slots:
    
    RLANN_GUI_BrushSizeButton*
    new_RLANN_GUI_BrushSizeButton(GUI_ItemBase* parent) {
        return new RLANN_GUI_BrushSizeButton(parent);
    }
    
};

#endif
