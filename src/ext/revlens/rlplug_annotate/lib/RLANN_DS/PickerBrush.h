

#ifndef EXTREVLENS_RLANN_PICKER_BRUSH_H
#define EXTREVLENS_RLANN_PICKER_BRUSH_H

#include "RlpExtrevlens/RLANN_DS/Global.h"
#include "RlpExtrevlens/RLANN_DS/BrushBase.h"
#include "RlpExtrevlens/RLANN_DS/Annotation.h"

class DISP_GLView;

class EXTREVLENS_RLANN_DS_API RLANN_DS_PickerBrush : public RLANN_DS_BrushBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    RLANN_DS_PickerBrush();

    void activate(DISP_GLView* view, RLANN_DS_DrawControllerBase* annCntrl);
    void setupFrame(RLANN_DS_AnnotationPtr ann);

    void setImage(QImage image);

    void strokeMove(QPointF& point, QVariantMap settings=QVariantMap());
    void strokeEnd();

    void paintGLPostDraw(GUI_Painter* painter, const QVariantMap& mdata);

public slots:

    QVariantMap settings();

    void onDisplayZoomChanged(float zoom);
    void onPlayStateChanged(int state);

private:
    QImage _image;

    QColor _col;
    QPointF _pos;
    QPointF _imgPos;
    QString _posText;
    QString _colText;

    float _zoom;
};

#endif