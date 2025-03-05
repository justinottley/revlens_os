
#ifndef GUI_PAINTER_BASE_H
#define GUI_PAINTER_BASE_H

#include "RlpGui/BASE/Global.h"

#include <qnanopainter.h>

#include <string>

class GUI_Painter {

public:
    enum PainterType {
        PAINTER_TYPE_QT,
        PAINTER_TYPE_QNANOPAINTER
    };


    RLP_DEFINE_LOGGER

    GUI_Painter(PainterType pt):
        _painterType(pt)
    {
    }

    PainterType getPainterType() { return _painterType; }

    virtual void setBrush(const QBrush& brush) = 0;
    virtual void setPen(const QPen& pen) = 0;
    virtual void setPen(const QColor& col) = 0;
    virtual void setFont(const QFont& font) = 0;
    virtual void setOpacity(qreal opacity) = 0;

    // virtual void setLinearGradient(const QLinearGradient& g) = 0;

    virtual const QPen& pen() = 0;
    virtual const QBrush& brush() = 0;
    virtual const QFont& font() = 0;
    virtual qreal opacity() = 0;

    // Drawing

    virtual void moveTo(qreal, qreal) {}
    virtual void cubicTo(qreal, qreal, qreal, qreal, qreal, qreal) {}
    virtual void strokeRect(const QRectF&) {}
    virtual void fillRect(const QRectF&) {}
    virtual void drawRect(const QRectF&) = 0;
    virtual void drawRect(int, int, int, int) = 0;
    virtual void drawImage(int, int, QImage*) = 0;
    virtual void drawImage(int, int, int, int, QImage*) = 0;
    virtual void drawText(int, int, const QString&) = 0;
    virtual void drawEllipse(qreal, qreal, qreal, qreal) = 0;
    virtual void drawRoundedRect(const QRectF&, qreal, qreal) = 0;
    virtual void drawLine(int, int, int, int) = 0;
    virtual void drawLine(const QPointF&, const QPointF&) = 0;
    virtual void drawPolygon(const QPoint*, int) = 0;
    virtual void drawCircle(qreal, qreal, qreal) {}
    virtual void strokePath(const QPainterPath&, const QPen&) = 0;

    // State

    virtual void beginPath() {}
    virtual void closePath() {}
    virtual void stroke() {}

    virtual void save() = 0;
    virtual void restore() = 0;
    virtual void reset() {}

    virtual void setRenderHint(QPainter::RenderHint hint, bool on = true) = 0;
    virtual void setCompositionMode(QPainter::CompositionMode mode) = 0;
    virtual void setClipRegion(const QRegion&) = 0;
    virtual void resetClipping() = 0;

protected:
    PainterType _painterType;

};



class GUI_QPainter : public GUI_Painter {

public:
    RLP_DEFINE_LOGGER

    GUI_QPainter(QPainter* p = nullptr);

    void setBrush(const QBrush& brush);
    void setPen(const QPen& pen);
    void setPen(const QColor& col);
    void setFont(const QFont& font);
    void setOpacity(qreal opacity);

    const QPen& pen();
    const QBrush& brush();
    const QFont& font(); 
    qreal opacity();

    // Drawing

    void drawRect(const QRectF&);
    void drawRect(int, int, int, int);
    void drawImage(int, int, QImage*);
    void drawImage(int, int, int, int, QImage*);
    void drawText(int, int, const QString&);
    void drawEllipse(qreal, qreal, qreal, qreal);
    void drawRoundedRect(const QRectF&, qreal, qreal);
    void drawLine(int, int, int, int);
    void drawLine(const QPointF&, const QPointF&);
    void drawPolygon(const QPoint*, int);
    void strokePath(const QPainterPath&, const QPen&);

    // State

    void save();
    void restore();

    void setRenderHint(QPainter::RenderHint hint, bool on = true);
    void setCompositionMode(QPainter::CompositionMode mode);
    void setClipRegion(const QRegion& region);
    void resetClipping();

    QPainter* getPainter() { return _p; }

private:
    QPainter* _p;

};

// -----------------

class GUI_NanoPainter : public GUI_Painter {

public:
    RLP_DEFINE_LOGGER

    GUI_NanoPainter(QNanoPainter* p);

    void setBrush(const QBrush& brush);
    void setPen(const QPen& pen);
    void setPen(const QColor& col);
    void setFont(const QFont& font);
    void setOpacity(qreal opacity);

    const QPen& pen();
    const QBrush& brush();
    const QFont& font(); 
    qreal opacity();

    // Drawing

    void moveTo(qreal, qreal);
    void cubicTo(qreal, qreal, qreal, qreal, qreal, qreal);
    void strokeRect(const QRectF&);
    void fillRect(const QRectF&);
    void drawRect(const QRectF&);
    void drawRect(int, int, int, int);
    void drawImage(int, int, QImage*);
    void drawImage(int, int, int, int, QImage*);
    void drawText(int, int, const QString&);
    void drawEllipse(qreal, qreal, qreal, qreal);
    void drawCircle(qreal, qreal, qreal);
    void drawRoundedRect(const QRectF&, qreal, qreal);
    void drawLine(int, int, int, int);
    void drawLine(const QPointF&, const QPointF&);
    void drawPolygon(const QPoint*, int);
    void strokePath(const QPainterPath&, const QPen&);

    // State

    void beginPath();
    void closePath();
    void stroke();

    void save();
    void restore();
    void reset();

    void setRenderHint(QPainter::RenderHint hint, bool on = true);
    void setCompositionMode(QPainter::CompositionMode mode);
    void setClipRegion(const QRegion& region);
    void resetClipping();
    
    // Compat

    QNanoPainter::LineJoin getLineJoin(const QPen& pen);
    QNanoPainter::LineCap getLineCap(const QPen& pen);

    QNanoImage* getNanoImage(QImage*);

private:
    QNanoPainter* _p;
    QPen _pen;
    QBrush _brush;
    QFont _qfont;
    QNanoFont _nfont;
    QNanoFont _nfontBold;

    qreal _opacity;

    static QMap<QString, QNanoImage*> IMAGE_CACHE;
};


#endif
