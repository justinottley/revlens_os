#ifndef EXTREVLENS_RLANN_DS_ERASER_BRUSH_H
#define EXTREVLENS_RLANN_DS_ERASER_BRUSH_H

#include "RlpExtrevlens/RLANN_DS/Global.h"

#include "RlpExtrevlens/RLANN_DS/PaintBrush.h"

class EXTREVLENS_RLANN_DS_API RLANN_DS_EraserBrush : public RLANN_DS_PaintBrush {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER
    
public:

    RLANN_DS_EraserBrush();

    void strokeMove(QPointF& point, QVariantMap settings=QVariantMap());
    void strokeTo(QPainter* painter, const QPointF& endPoint);
    void updateSettings(QVariantMap settings);

    void paintGLPostDraw(GUI_Painter* painter, const QVariantMap& mdata);
};

#endif