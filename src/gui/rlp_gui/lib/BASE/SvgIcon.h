//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef CORE_GUI_SVG_ICON_H
#define CORE_GUI_SVG_ICON_H

#include "RlpGui/BASE/Global.h"
#include "RlpGui/BASE/Style.h"
#include "RlpGui/BASE/ItemBase.h"

#include <QtSvg/QSvgRenderer>


class GUI_BASE_API GUI_SvgUtil : public QObject {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    GUI_SvgUtil();

public slots:

    static GUI_SvgUtil*
    new_GUI_SvgUtil()
    {
        return new GUI_SvgUtil();
    }

    QImage renderSvg(QString svgPath, int size);

};

class GUI_BASE_API GUI_SvgIcon : public GUI_ItemBase {
    Q_OBJECT

signals:
    void hoverChanged(bool val);
    void buttonPressed(QVariantMap metadata);
    void forwardMouseEvent(QMouseEvent* event);

public:
    RLP_DEFINE_LOGGER

    GUI_SvgIcon(QString filename,
                GUI_ItemBase* parent,
                qreal size=18,
                qreal padding=0,
                QColor bgCol=GUI_Style::IconBg,
                bool changeColour=true,
                GUI_ItemBase::PaintMode paintMode=GUI_ItemBase::PAINT_NANOPAINTER
    );

    void mousePressEvent(QMouseEvent* event);
    void hoverEnterEvent(QHoverEvent* event);
    void hoverLeaveEvent(QHoverEvent* event);
    
    QRectF boundingRect() const;

    void paint(GUI_Painter* painter);

    QImage* bgImage() { return &_bgImage; }
    QImage* fgImage() { return &_fgImage; }

    static QImage loadSvgFromPath(QString iconPath, int size, QColor colour, bool changeColour=true);

public slots:

    static GUI_SvgIcon*
    new_GUI_SvgIcon(QString filename, GUI_ItemBase* parent, qreal size, qreal padding, QColor col)
    {
        return new GUI_SvgIcon(filename, parent, size, padding, col);
    }

    void setBgColour(QColor col);
    void setFgColour(QColor col);

    void setOutlined(bool outlined) { _doOutlined = outlined; }
    void setOutlinedBgColour(QColor col) { _outlineBgCol = col ; }

    void setBgImage(QImage img) { _bgImage = img; }
    void setFgImage(QImage img) { _fgImage = img; }

    bool inHover() { return _inHover; }
    void setHover(bool hover);
    void setHoverColour(QColor col) { setFgColour(col); }
    void setForwardMouseEvent(bool doForward) { _doForwardMouseEvent = doForward; }

    QString filename() { return _filename; }

    void setXOffset(int x) { _xoff = x; }
    void setYOffset(int y) { _yoff = y; }

private:

    QString _filename;
    int _size;
    qreal _padding;

    int _xoff;
    int _yoff;

    bool _inHover;
    bool _doOutlined;

    QColor _bgCol;
    QColor _fgCol;

    QColor _outlineBgCol;

    QImage _bgImage;
    QImage _fgImage;

    bool _doForwardMouseEvent;
};

class GUI_PaintedSvgIcon : public GUI_SvgIcon {
    Q_OBJECT

public:
    GUI_PaintedSvgIcon(
        QString filename,
        GUI_ItemBase* parent,
        qreal size=18,
        qreal padding=0
    ):
        GUI_SvgIcon(
            filename,
            parent,
            size,
            padding,
            GUI_Style::IconBg,
            true,
            GUI_ItemBase::PAINT_QT
        )
    {
        qInfo() << "PAINTED SVG ICON!!!!";
    }

public slots:

    static GUI_PaintedSvgIcon* new_GUI_PaintedSvgIcon(
        QString filename, GUI_ItemBase* parent, qreal size
    )
    {
        return new GUI_PaintedSvgIcon(filename, parent, size);
    }
};


#endif