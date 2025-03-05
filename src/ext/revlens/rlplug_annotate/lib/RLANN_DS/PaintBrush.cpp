
#include "RlpExtrevlens/RLANN_DS/PaintBrush.h"

RLP_SETUP_EXT_LOGGER(RLANN_DS, PaintBrush)

RLANN_DS_PaintBrush::RLANN_DS_PaintBrush(QString name):
    RLANN_DS_BrushBase(name),
    _doAntialiasing(true),
    _penCol(200, 20, 20),
    _penWidth(2),
    _brush(Qt::transparent)
{ 
}


QVariantMap
RLANN_DS_PaintBrush::settings()
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
RLANN_DS_PaintBrush::updateSettings(QVariantMap settings)
{
    if (settings.contains("col")) {
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
    
    if (settings.contains("width")) {
        _penWidth = settings.value("width").toInt();
    }
    
    // RLP_LOG_DEBUG("pen width: " << _penWidth);
}


void
RLANN_DS_PaintBrush::strokeBegin(QPointF& point, QVariantMap settings)
{
    RLP_LOG_DEBUG("");

    _lastPoint = point;
    updateSettings(settings);


}


void
RLANN_DS_PaintBrush::strokeMove(QPointF& point, QVariantMap extra)
{
    if (_playstate == 0) // playing
    {
        return;
    }

    _view->setCursor(Qt::BlankCursor);
    _view->update();
}


void
RLANN_DS_PaintBrush::strokeTo(QPainter* painter, const QPointF &endPoint)
{
    // RLP_LOG_DEBUG(_penWidth << " " << _penCol.red() << " " << _penCol.green() << " " << _penCol.blue());
    
    if (_doAntialiasing) {
        painter->setRenderHint(QPainter::Antialiasing, true);
    }

    QPen pen(
        _ppenCol,
        _penWidth,
        Qt::SolidLine,
        Qt::RoundCap,
        Qt::RoundJoin);

    painter->setPen(pen);
    painter->setBrush(_brush);
    painter->drawLine(_lastPoint, endPoint);

    _lastPoint = endPoint;
}


void
RLANN_DS_PaintBrush::paintGLPostDraw(GUI_Painter* painter, const QVariantMap& mdata)
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
            _penWidth / 2
        );

        painter->restore();
    }
}
