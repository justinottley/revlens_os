
#include "RlpExtrevlens/RLANN_BRUSH/EraserBrush.h"

RLP_SETUP_LOGGER(ext, RlpAnnBrush, Eraser)

RlpAnnBrush_Eraser::RlpAnnBrush_Eraser():
    RlpAnnBrush_Paint("BasicEraser")
{
    _penCol = Qt::transparent;
}


RLANN_DS_BrushBase*
RlpAnnBrush_Eraser::clone()
{
    return new RlpAnnBrush_Eraser();
}


void
RlpAnnBrush_Eraser::updateSettings(QVariantMap settings)
{
    // RLP_LOG_DEBUG("");

    if (settings.contains("width"))
    {
        _penWidth = settings.value("width").toInt();
    }
}


void
RlpAnnBrush_Eraser::strokeMove(QPointF& point, QVariantMap mdata)
{
    if (_playstate == 0) // playing
    {
        return;
    }

    // _view->setCursor(Qt::BlankCursor);
    _view->update();
}

void
RlpAnnBrush_Eraser::strokeTo(RLANN_DS_AnnotationPtr ann, const QPointF &endPoint, QVariantMap mdata)
{
    // RLP_LOG_DEBUG(_penWidth << " " << _penCol.red() << " " << _penCol.green() << " " << _penCol.blue());

    QPainter painter(ann->getQImage());

    if (_doAntialiasing)
    {
        painter.setRenderHint(QPainter::Antialiasing, true);
    }

    QPen pen(
        _penCol,
        std::max(10, _penWidth * 2),
        Qt::SolidLine,
        Qt::RoundCap,
        Qt::RoundJoin);

    painter.setCompositionMode(QPainter::CompositionMode_Source);

    painter.setPen(pen);
    painter.setBrush(_brush);
    painter.drawLine(_lastPoint, endPoint);

    _lastPoint = endPoint;
}


void
RlpAnnBrush_Eraser::paintGLPostDraw(GUI_Painter* painter, const QVariantMap& mdata)
{
    if (_view != nullptr)
    {
        QPointF ccpos = _view->mapFromGlobal(QCursor::pos());

        painter->setPen(Qt::black);
        painter->setBrush(QBrush(Qt::transparent));
        painter->drawCircle(
            ccpos.x() +  1,
            ccpos.y() + 1,
            std::max(10, _penWidth - 4)
        );

        painter->setPen(Qt::white);
        painter->setBrush(QBrush(Qt::transparent));
        painter->drawCircle(
            ccpos.x(),
            ccpos.y(),
            std::max(10, _penWidth - 4)
        );
    }
}