#ifndef EXTREVLENS_RLANN_BRUSH_ERASER_H
#define EXTREVLENS_RLANN_BRUSH_ERASER_H

#include "RlpExtrevlens/RLANN_BRUSH/Global.h"

#include "RlpExtrevlens/RLANN_BRUSH/PaintBrush.h"

class EXTREVLENS_RLANN_BRUSH_API RlpAnnBrush_Eraser : public RlpAnnBrush_Paint {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER
    
public:

    RlpAnnBrush_Eraser();
    RLANN_DS_BrushBase* clone();

    void strokeMove(QPointF& point, QVariantMap mdata=QVariantMap());
    void strokeTo(RLANN_DS_AnnotationPtr ann, const QPointF& endPoint, QVariantMap mdata=QVariantMap());
    void updateSettings(QVariantMap settings);

    void paintGLPostDraw(GUI_Painter* painter, const QVariantMap& mdata);
};

#endif