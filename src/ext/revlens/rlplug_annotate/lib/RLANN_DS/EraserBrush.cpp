
#include "RlpExtrevlens/RLANN_DS/EraserBrush.h"

RLP_SETUP_EXT_LOGGER(RLANN_DS, EraserBrush)

RLANN_DS_EraserBrush::RLANN_DS_EraserBrush():
    RLANN_DS_PaintBrush("BasicEraser")
{
    _penCol = Qt::transparent;
}


void
RLANN_DS_EraserBrush::updateSettings(QVariantMap settings)
{
    // RLP_LOG_DEBUG("");

    if (settings.contains("width")) {
        _penWidth = settings.value("width").toInt();
    }
}


void
RLANN_DS_EraserBrush::strokeMove(QPointF& point, QVariantMap extra)
{
    if (_playstate == 0) // playing
    {
        return;
    }

    _view->setCursor(Qt::BlankCursor);
    _view->update();
}

void
RLANN_DS_EraserBrush::strokeTo(QPainter* painter, const QPointF &endPoint)
{
    // RLP_LOG_DEBUG(_penWidth << " " << _penCol.red() << " " << _penCol.green() << " " << _penCol.blue());
    
    if (_doAntialiasing) {
        painter->setRenderHint(QPainter::Antialiasing, true);
    }


    QPen pen(
        _penCol,
        std::max(10, _penWidth * 2),
        Qt::SolidLine,
        Qt::RoundCap,
        Qt::RoundJoin);

    painter->setCompositionMode(QPainter::CompositionMode_Source);

    painter->setPen(pen);
    painter->setBrush(_brush);
    painter->drawLine(_lastPoint, endPoint);

    _lastPoint = endPoint;
}


void
RLANN_DS_EraserBrush::paintGLPostDraw(GUI_Painter* painter, const QVariantMap& mdata)
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