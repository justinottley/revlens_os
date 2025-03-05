//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef EXTREVLENS_RLANN_DS_TEXT_BRUSH_H
#define EXTREVLENS_RLANN_DS_TEXT_BRUSH_H

#include "RlpExtrevlens/RLANN_DS/Global.h"
#include "RlpExtrevlens/RLANN_DS/BrushBase.h"

#include "RlpGui/WIDGET/TextEdit.h"
#include "RlpGui/WIDGET/IconButton.h"
#include "RlpGui/PANE/PaneFrame.h"

class DISP_GLView;

class EXTREVLENS_RLANN_DS_API RLANN_DS_TextBrush : public RLANN_DS_BrushBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    RLANN_DS_TextBrush(QString name="Text");

    void activate(DISP_GLView* view, RLANN_DS_DrawControllerBase* annCntrl);
    void setupFrame(RLANN_DS_AnnotationPtr ann);
    void strokeBegin(QPointF& point, QVariantMap settings = QVariantMap());
    void strokeTo(QPainter* painter, const QPointF& endPoint);

    void updateSettings(QVariantMap settings);
    QVariantMap settings();

public slots:

    void onCreateTextAnnotation(QVariantMap md=QVariantMap());
    void onCloseTextPaneRequested();

protected:

    QPointF _lastPoint;
    QColor _penCol;
    int _penWidth;
    QBrush _brush;
    bool _doAntialiasing;

    DISP_GLView* _view;
    RLANN_DS_AnnotationPtr _ann;

    GUI_PaneFrame* _pf;
    GUI_TextEdit* _te;
};

#endif