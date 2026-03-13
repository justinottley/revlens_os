
#ifndef RLANN_MP_ERASER_BRUSH_H
#define RLANN_MP_ERASER_BRUSH_H

#include "RlpExtrevlens/RLANN_MP/Global.h"
#include "RlpExtrevlens/RLANN_MP/Brush.h"

/*
 * This came about because the MyPaint eraser brushes appear to leave behind
 * pixel artifacts - they dont erase completely, for unknown reason.
 * This brush ensures clean erasing
 *
 */
class EXTREVLENS_RLANN_MP_API RLANN_MP_EraserBrush : public RLANN_MP_Brush {

public:
    RLP_DEFINE_LOGGER

    RLANN_MP_EraserBrush(QString name="MyPaintEraser");
    RLANN_DS_BrushBase* clone() { return new RLANN_MP_EraserBrush(); }

    void strokeTo(RLANN_DS_AnnotationPtr ann, const QPointF& endPoint, QVariantMap mdata=QVariantMap());

protected:
    QBrush _qbrush;

};

#endif
