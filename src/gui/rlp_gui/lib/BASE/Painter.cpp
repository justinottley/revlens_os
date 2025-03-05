
#include "RlpGui/BASE/Painter.h"
#include "RlpGui/BASE/Style.h"

#include <iostream>
#include <sstream>

RLP_SETUP_LOGGER(gui, GUI, Painter)
RLP_SETUP_LOGGER(gui, GUI, QPainter)
RLP_SETUP_LOGGER(gui, GUI, NanoPainter)

GUI_QPainter::GUI_QPainter(QPainter* p):
    GUI_Painter(GUI_Painter::PAINTER_TYPE_QT),
    _p(p)
{
}

void
GUI_QPainter::setBrush(const QBrush& brush)
{
    _p->setBrush(brush);
}

void
GUI_QPainter::setPen(const QPen& pen)
{
    _p->setPen(pen);
}

void
GUI_QPainter::setPen(const QColor& col)
{
    _p->setPen(col);
}

void
GUI_QPainter::setFont(const QFont& font)
{
    _p->setFont(font);
}

void
GUI_QPainter::setOpacity(qreal opacity)
{
    _p->setOpacity(opacity);
}

const QPen&
GUI_QPainter::pen()
{
    return _p->pen();
}

const QBrush&
GUI_QPainter::brush()
{
    return _p->brush();
}

const QFont&
GUI_QPainter::font()
{
    return _p->font();
}

qreal
GUI_QPainter::opacity()
{
    return _p->opacity();
}

void
GUI_QPainter::drawRect(const QRectF& rect)
{
    _p->drawRect(rect);
}

void
GUI_QPainter::drawRect(int x, int y, int w, int h)
{
    _p->drawRect(x, y, w, h);
}

void
GUI_QPainter::drawImage(int x, int y, QImage* image)
{
    _p->drawImage(x, y, *image);
}

void
GUI_QPainter::drawImage(int x, int y, int w, int h, QImage* image)
{
    QRect r(x, y, w, h);
    _p->drawImage(r, *image);
}

void
GUI_QPainter::drawText(int x, int y, const QString& text)
{
    _p->drawText(x, y, text);
}

void
GUI_QPainter::drawEllipse(qreal x, qreal y, qreal t, qreal z)
{
    _p->drawEllipse(x, y, t, z);
}

void
GUI_QPainter::drawRoundedRect(const QRectF& rect, qreal xr, qreal yr)
{
    _p->drawRoundedRect(rect, xr, yr);
}

void
GUI_QPainter::drawLine(int x, int y, int dx, int dy)
{
    _p->drawLine(x, y, dx, dy);
}

void
GUI_QPainter::drawLine(const QPointF& src, const QPointF& dst)
{
    _p->drawLine(src, dst);
}

void
GUI_QPainter::drawPolygon(const QPoint* points, int pointCount)
{
    _p->drawPolygon(points, pointCount);
}


void
GUI_QPainter::strokePath(const QPainterPath& path, const QPen& pen)
{
    _p->strokePath(path, pen);
}

void
GUI_QPainter::save()
{
    _p->save();
}

void
GUI_QPainter::restore()
{
    _p->restore();
}

void
GUI_QPainter::setClipRegion(const QRegion& region)
{
    _p->setClipRegion(region);
}

void
GUI_QPainter::resetClipping()
{
    // _p->resetClipping();
}

void
GUI_QPainter::setCompositionMode(QPainter::CompositionMode mode)
{
    _p->setCompositionMode(mode);
}

void
GUI_QPainter::setRenderHint(QPainter::RenderHint hint, bool on)
{
    _p->setRenderHint(hint, on);
}


// ----------------------


QMap<QString, QNanoImage*> GUI_NanoPainter::IMAGE_CACHE;

GUI_NanoPainter::GUI_NanoPainter(QNanoPainter* p):
    GUI_Painter(GUI_Painter::PAINTER_TYPE_QNANOPAINTER),
    _p(p),
    _pen(),
    _brush(),
    _qfont(QApplication::font()),
    _nfont(":misc/font_nanum_gothic.ttf"),
    _nfontBold(":misc/font_nanum_gothic_bold.ttf"),
    _opacity(1.0)
{
    _nfont.setPixelSize(GUI_Style::FontPixelSizeDefault);
    _nfontBold.setPixelSize(GUI_Style::FontPixelSizeDefault);
    _qfont.setPixelSize(GUI_Style::FontPixelSizeDefault);
    _p->setFont(_nfont);
}


void
GUI_NanoPainter::setBrush(const QBrush& brush)
{
    const QGradient* gr = brush.gradient();
    if (gr == nullptr)
    {
        _p->setFillStyle(QNanoColor::fromQColor(brush.color()));
    } else
    {
        if (gr->type() == QGradient::LinearGradient)
        {
            const QLinearGradient* lg = static_cast<const QLinearGradient*>(gr);
            
            QPointF start = lg->start();
            QPointF stop = lg->finalStop();

            QList<QGradientStop> stops = gr->stops();
            QPair<qreal, QColor> startItem = stops.at(0);
            QPair<qreal, QColor> endItem = stops.at(1);

            QNanoLinearGradient nlg(start, stop);
            nlg.setStartColor(QNanoColor::fromQColor(startItem.second));
            nlg.setEndColor(QNanoColor::fromQColor(endItem.second));

            _p->setFillStyle(nlg);
        }
    }

    _brush = brush;
}


void
GUI_NanoPainter::setPen(const QPen& pen)
{
    _p->setLineWidth(pen.width());
    _p->setLineCap(getLineCap(pen));
    _p->setLineJoin(getLineJoin(pen));
    
    QNanoColor penCol(QNanoColor::fromQColor(pen.color()));

    _p->setStrokeStyle(penCol);
    _p->setFillStyle(penCol);

    _pen = pen;
}

void
GUI_NanoPainter::setPen(const QColor& col)
{
    setPen(QPen(col));
}

void
GUI_NanoPainter::setFont(const QFont& font)
{
    // RLP_LOG_DEBUG(font.pixelSize())

    _qfont = font;

    // TODO FIXME: better support for italic, get font based on bold or italic

    if (font.bold())
    {
        _nfontBold.setPixelSize(font.pixelSize());
        _p->setFont(_nfontBold);

    } else
    {
        _nfont.setPixelSize(font.pixelSize());
        _p->setFont(_nfont);
    }
    
}

void
GUI_NanoPainter::setOpacity(qreal opacity)
{
    _p->setGlobalAlpha(opacity);

    _opacity = opacity;
}

const QPen&
GUI_NanoPainter::pen()
{
    return _pen;
}

const QBrush&
GUI_NanoPainter::brush()
{
    return _brush;
}

const QFont&
GUI_NanoPainter::font()
{
    return _qfont;
}

qreal
GUI_NanoPainter::opacity()
{
    return _opacity;
}


// Compat

QNanoPainter::LineJoin
GUI_NanoPainter::getLineJoin(const QPen& pen)
{
    switch (pen.joinStyle())
    {
        case Qt::MiterJoin:
            return QNanoPainter::JOIN_MITER;
        case Qt::BevelJoin:
            return QNanoPainter::JOIN_BEVEL;
        case Qt::RoundJoin:
            return QNanoPainter::JOIN_ROUND;
        default:
            return QNanoPainter::JOIN_ROUND;

    }
}


QNanoPainter::LineCap
GUI_NanoPainter::getLineCap(const QPen& pen)
{
    switch (pen.capStyle())
    {
        case Qt::FlatCap:
            return QNanoPainter::CAP_BUTT;
        case Qt::SquareCap:
            return QNanoPainter::CAP_SQUARE;
        case Qt::RoundCap:
            return QNanoPainter::CAP_ROUND;
        default:
            return QNanoPainter::CAP_SQUARE;
    }
}


QNanoImage*
GUI_NanoPainter::getNanoImage(QImage* image)
{
    QNanoImage* ni = nullptr;

    QString imgId = image->text("id");
    if (imgId.isEmpty())
    {
        imgId = QUuid::createUuid().toString(QUuid::Id128);
        // RLP_LOG_DEBUG("Created image ID:" << imgId)
    
        image->setText("id", imgId);
    }


    if (!GUI_NanoPainter::IMAGE_CACHE.contains(imgId))
    {
        // RLP_LOG_DEBUG("Caching QImage at" << imgId)

        ni = new QNanoImage(*image, imgId);
        GUI_NanoPainter::IMAGE_CACHE.insert(imgId, ni);

    } else
    {
        ni = GUI_NanoPainter::IMAGE_CACHE.value(imgId);
    }

    return ni;
}

// Drawing


void
GUI_NanoPainter::moveTo(qreal x, qreal y)
{
    _p->moveTo(x, y);
}

void
GUI_NanoPainter::cubicTo(qreal c1x, qreal c1y, qreal c2x, qreal c2y, qreal x, qreal y)
{
    _p->bezierTo(c1x, c1y, c2x, c2y, x, y);
}

void
GUI_NanoPainter::strokeRect(const QRectF& rect)
{
    _p->strokeRect(rect);
}

void
GUI_NanoPainter::fillRect(const QRectF& rect)
{
    _p->fillRect(rect);
}

void
GUI_NanoPainter::drawRect(const QRectF& rect)
{
    _p->fillRect(rect);
    _p->strokeRect(rect);
}


void
GUI_NanoPainter::drawRect(int x, int y, int w, int h)
{
    _p->fillRect(x, y, w, h);
    _p->strokeRect(x, y, w, h);
}


void
GUI_NanoPainter::drawImage(int x, int y, QImage* image)
{
    // QString imgId = QUuid::createUuid().toString(QUuid::Id128);
    // QNanoImage n(*image, imgId);
    // _p->drawImage(n, x, y);

    QNanoImage* ni = getNanoImage(image);
    if (ni != nullptr)
    {
        _p->drawImage(*ni, x, y);
    }
}


void
GUI_NanoPainter::drawImage(int x, int y, int w, int h, QImage* image)
{
    QNanoImage* ni = getNanoImage(image);
    if (ni != nullptr)
    {
        _p->drawImage(*ni, x, y, w, h);
    }
}

void
GUI_NanoPainter::drawText(int x, int y, const QString& text)
{
    _p->fillText(text, x, y);
}

void
GUI_NanoPainter::drawEllipse(qreal cx, qreal cy, qreal rx, qreal ry)
{
    _p->beginPath();
    _p->ellipse(cx, cy, rx, ry);
    _p->fill();
    _p->stroke();
    _p->closePath();
}

void
GUI_NanoPainter::drawCircle(qreal cx, qreal cy, qreal r)
{
    _p->beginPath();
    _p->circle(cx, cy, r);
    _p->fill();
    _p->stroke();
    _p->closePath();
}


void
GUI_NanoPainter::drawRoundedRect(const QRectF& rect, qreal x, qreal)
{
    _p->beginPath();
    _p->roundedRect(rect, x);
    _p->fill();
    _p->stroke();
    _p->closePath();
}



void
GUI_NanoPainter::drawLine(int x, int y, int w, int h)
{
    _p->beginPath();
    _p->moveTo(x, y);
    _p->lineTo(w, h);
    _p->fill();
    _p->stroke();
    _p->closePath();
}


void
GUI_NanoPainter::drawLine(const QPointF& pos, const QPointF& end)
{
    _p->beginPath();
    _p->moveTo(pos.x(), pos.y());
    _p->lineTo(end.x(), end.y());
    _p->stroke();
    _p->closePath();
}

void
GUI_NanoPainter::drawPolygon(const QPoint* points, int pointCount)
{
    _p->beginPath();
    QPointF firstPoint = points[0];
    _p->moveTo(firstPoint.x(), firstPoint.y() - 1);
    _p->lineTo(firstPoint.x(), firstPoint.y());

    for (int i=1; i != pointCount; ++i)
    {
        _p->lineTo(points[i].x(), points[i].y());
    }
    _p->stroke();
    _p->fill();
    _p->closePath();
}

void
GUI_NanoPainter::strokePath(const QPainterPath& pp, const QPen& pen)
{
    RLP_LOG_ERROR("Not implemented")
}


// State

void
GUI_NanoPainter::beginPath()
{
    _p->beginPath();
}

void
GUI_NanoPainter::closePath()
{
    _p->closePath();
}

void
GUI_NanoPainter::stroke()
{
    _p->stroke();
}

void 
GUI_NanoPainter::save()
{
    _p->save();
}

void
GUI_NanoPainter::restore()
{
    _p->restore();
}

void
GUI_NanoPainter::reset()
{
    _p->reset();
}

void
GUI_NanoPainter::setRenderHint(QPainter::RenderHint hint, bool on)
{
    RLP_LOG_WARN("not implemented")
}

void
GUI_NanoPainter::setCompositionMode(QPainter::CompositionMode mode)
{
    switch(mode)
    {
        case QPainter::CompositionMode_Source:
        {
            _p->setGlobalCompositeOperation(QNanoPainter::COMPOSITE_SOURCE_OVER);
        }
        default:
        {
            RLP_LOG_ERROR("Other composite modes not supported")
        }
    }
}

void
GUI_NanoPainter::setClipRegion(const QRegion& region)
{
    _p->setClipRect(region.boundingRect());
}

void
GUI_NanoPainter::resetClipping()
{
    _p->resetClipping();
}


