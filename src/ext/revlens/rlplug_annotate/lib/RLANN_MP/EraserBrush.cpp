
#include "RlpExtrevlens/RLANN_MP/EraserBrush.h"
#include "RlpExtrevlens/RLANN_MP/Handler.h"
#include "RlpExtrevlens/RLANN_MP/Surface.h"
#include "RlpExtrevlens/RLANN_MP/Tile.h"

RLP_SETUP_LOGGER(ext, RLANN_MP, EraserBrush)

RLANN_MP_EraserBrush::RLANN_MP_EraserBrush(QString name):
    RLANN_MP_Brush(name),
    _qbrush(Qt::transparent)
{
}

void
RLANN_MP_EraserBrush::strokeTo(RLANN_DS_AnnotationPtr ann, const QPointF& endPoint, QVariantMap mdata)
{
    // RLP_LOG_DEBUG("")

    QPainter painter(ann->getQImage());

    if (_doAntialiasing)
    {
        painter.setRenderHint(QPainter::Antialiasing, true);
    }

    QPen pen(
        Qt::transparent,
        std::max(10, _penWidth * 2),
        Qt::SolidLine,
        Qt::RoundCap,
        Qt::RoundJoin);

    painter.setCompositionMode(QPainter::CompositionMode_Source);

    painter.setPen(pen);
    painter.setBrush(_qbrush);
    painter.drawLine(_lastPoint, endPoint);

    _lastPoint = endPoint;

    RLANN_MP_Surface* surf = RLANN_MP_Handler::instance()->getSurface(_currUuid);
    if (surf == nullptr)
    {
        RLP_LOG_ERROR("No RLANN_MP surface")
        return;
    }

    RLANN_MP_Tile* tile = surf->getTileByPos(QPoint(_lastPoint.x(), _lastPoint.y()));
    tile->updateTileFromImage(ann->getQImage(), /* alphaValue = */ 0);
}
