

#ifndef EXTREVLENS_RLANN_BRUSH_PICKER_H
#define EXTREVLENS_RLANN_BRUSH_PICKER_H

#include "RlpExtrevlens/RLANN_BRUSH/Global.h"

#include "RlpExtrevlens/RLANN_DS/BrushBase.h"
#include "RlpExtrevlens/RLANN_DS/Annotation.h"

class DISP_GLView;

class EXTREVLENS_RLANN_BRUSH_API RlpAnnBrush_Picker : public RLANN_DS_BrushBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    RlpAnnBrush_Picker();

    RLANN_DS_BrushBase* clone();

    void activate(DISP_GLView* view, RLANN_DS_DrawControllerBase* annCntrl);
    void setupFrame(RLANN_DS_AnnotationPtr ann, QVariantMap mdata);

    void setImage(QImage image);

    void strokeMove(QPointF& point, QVariantMap mdata=QVariantMap());
    // void strokeEnd();

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
    int _dpr; // device pixel ratio
    int _woffset;
};

#endif