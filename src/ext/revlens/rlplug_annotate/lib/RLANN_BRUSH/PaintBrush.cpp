
#include "RlpExtrevlens/RLANN_BRUSH/PaintBrush.h"

RLP_SETUP_LOGGER(ext, RlpAnnBrush, Paint)

RlpAnnBrush_Paint::RlpAnnBrush_Paint(QString name):
    RLANN_DS_BrushBase(name),
    _doAntialiasing(true),
    _penCol(200, 20, 20),
    _penWidth(2),
    _brush(Qt::transparent),
    _style(BRUSH_STYLE_HARD)
{ 
}


RLANN_DS_BrushBase*
RlpAnnBrush_Paint::clone()
{
    return new RlpAnnBrush_Paint();
}


QVariantMap
RlpAnnBrush_Paint::settings()
{
    QVariantMap result;
    QVariantMap col;
    col.insert("r", _penCol.red());
    col.insert("g", _penCol.green());
    col.insert("b", _penCol.blue());
    result.insert("col", col);
    result.insert("width", _penWidth);
    result.insert("name", _name);
    
    return result;

}


void
RlpAnnBrush_Paint::updateSettings(QVariantMap settings)
{
    if (settings.contains("col"))
    {
        QVariantMap col = settings.value("col").toMap();
        _penCol = QColor(
            col.value("r").toInt(),
            col.value("g").toInt(),
            col.value("b").toInt()
        );
        _ppenCol = QColor(
            col.value("b").toInt(),
            col.value("g").toInt(),
            col.value("r").toInt()
        );

    }
    
    if (settings.contains("width"))
    {
        _penWidth = settings.value("width").toInt();
    }
    
    // RLP_LOG_DEBUG("pen width: " << _penWidth);
}


void
RlpAnnBrush_Paint::strokeBegin(QPointF& point, QVariantMap mdata)
{
    RLP_LOG_DEBUG("");

    _lastPoint = point;
    if (mdata.contains("settings"))
    {
        updateSettings(mdata.value("settings").toMap());
    }


}


void
RlpAnnBrush_Paint::strokeMove(QPointF& point, QVariantMap extra)
{
    if (_playstate == 0) // playing
    {
        return;
    }

    // _view->setCursor(Qt::BlankCursor);
    _view->update();
}


void
RlpAnnBrush_Paint::strokeTo(RLANN_DS_AnnotationPtr ann, const QPointF &endPoint, QVariantMap mdata)
{
    // RLP_LOG_DEBUG("")

    QPainter painter(ann->getQImage());

    if (_style == BRUSH_STYLE_HARD)
    {
        strokeToHard(&painter, endPoint, mdata);
    } else {
        strokeToSoft(&painter, endPoint, mdata);
    }
}


void
RlpAnnBrush_Paint::strokeEnd(RLANN_DS_AnnotationPtr ann, QVariantMap mdata)
{
    RLP_LOG_DEBUG("")
    // _path.clear();
}


void
RlpAnnBrush_Paint::strokeToSoft(QPainter* painter, const QPointF &endPoint, QVariantMap mdata)
{
    // RLP_LOG_DEBUG(_penWidth << " " << _penCol.red() << " " << _penCol.green() << " " << _penCol.blue());
    RLP_LOG_DEBUG(endPoint.x() << " " << endPoint.y());

    if (_doAntialiasing)
    {
        painter->setRenderHint(QPainter::Antialiasing, true);
    }

    QRadialGradient rg(
        endPoint,
        _penWidth
    );

    rg.setColorAt(0.0, _penCol);
    rg.setColorAt(1.0, Qt::transparent);

    QBrush b(rg);

    painter->setPen(Qt::NoPen);
    // painter->setOpacity(0.25);
    painter->setBrush(b);

    // painter->setCompositionMode(QPainter::CompositionMode_Exclusion);

    // _path.addEllipse(endPoint, _penWidth, _penWidth);
    // _path.lineTo(endPoint);
    // painter->drawPath(_path);

    painter->drawEllipse(
        endPoint,
        _penWidth,
        _penWidth
    );

    _lastPoint = endPoint;
}


void
RlpAnnBrush_Paint::strokeToHard(QPainter* painter, const QPointF &endPoint, QVariantMap mdata)
{
    painter->setRenderHint(QPainter::Antialiasing, true);

    QPen pen(
        _ppenCol,
        _penWidth * 2,
        Qt::SolidLine,
        Qt::RoundCap,
        Qt::RoundJoin
    );

    painter->setPen(pen);
    painter->setBrush(_brush);

    painter->drawLine(_lastPoint, endPoint);
    _lastPoint = endPoint;
}


void
RlpAnnBrush_Paint::paintGLPostDraw(GUI_Painter* painter, const QVariantMap& mdata)
{
    if (_view != nullptr)
    {
        QPointF ccpos = _view->mapFromGlobal(QCursor::pos());

        painter->save();
        painter->setPen(_penCol);
        painter->setBrush(QBrush(Qt::transparent));

        painter->drawCircle(
            ccpos.x(),
            ccpos.y(),
            _penWidth
        );

        painter->restore();
    }
}
