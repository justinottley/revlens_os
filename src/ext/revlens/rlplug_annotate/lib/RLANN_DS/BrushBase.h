//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef EXTREVLENS_RLANN_DS_BRUSH_H
#define EXTREVLENS_RLANN_DS_BRUSH_H

#include "RlpExtrevlens/RLANN_DS/Global.h"

#include "RlpExtrevlens/RLANN_DS/Annotation.h"
#include "RlpExtrevlens/RLANN_DS/DrawControllerBase.h"

#include "RlpRevlens/DISP/GLView.h"

#include "RlpGui/BASE/Painter.h"

class DISP_GLView;

class EXTREVLENS_RLANN_DS_API RLANN_DS_BrushBase : public QObject {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    RLANN_DS_BrushBase(QString name);

    virtual void activate(DISP_GLView* view, RLANN_DS_DrawControllerBase* annCntrl);

public slots:

    QString name() { return _name; }

    virtual void updateSettings(QVariantMap settings) {}

    virtual void strokeBegin(QPointF& point, QVariantMap extra=QVariantMap()) {}
    virtual void strokeMove(QPointF& point, QVariantMap extra=QVariantMap()) {}
    virtual void strokeTo(QPainter* painter, const QPointF& endPoint) {}
    virtual void strokeEnd(RLANN_DS_AnnotationPtr ann);

    virtual void paintGLPostDraw(GUI_Painter* painter, const QVariantMap& mdata) {}
    virtual QVariantMap settings() { return QVariantMap(); }

    virtual void setupFrame(RLANN_DS_AnnotationPtr ann);
    

    virtual void onPlayStateChanged(int state);


protected:

    QString _name;

    DISP_GLView* _view;
    RLANN_DS_DrawControllerBase* _annCntrl;

    bool _connected;
    int _playstate;


};

#endif