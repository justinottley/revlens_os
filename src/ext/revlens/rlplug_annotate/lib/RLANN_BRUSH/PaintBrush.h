//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef EXTREVLENS_RLANN_BRUSH_PAINT_H
#define EXTREVLENS_RLANN_BRUSH_PAINT_H


#include "RlpExtrevlens/RLANN_BRUSH/Global.h"
#include "RlpExtrevlens/RLANN_DS/BrushBase.h"

class EXTREVLENS_RLANN_BRUSH_API RlpAnnBrush_Paint : public RLANN_DS_BrushBase {

public:
    enum BrushStyle {
        BRUSH_STYLE_NONE,
        BRUSH_STYLE_HARD,
        BRUSH_STYLE_SOFT
    };


public:
    RLP_DEFINE_LOGGER

    RlpAnnBrush_Paint(QString name="BasicPaint");
    RLANN_DS_BrushBase* clone();

    void strokeBegin(QPointF& point, QVariantMap mdata=QVariantMap());
    void strokeMove(QPointF& point, QVariantMap mdata=QVariantMap());
    void strokeTo(RLANN_DS_AnnotationPtr ann, const QPointF& endPoint, QVariantMap mdata=QVariantMap());
    void strokeEnd(RLANN_DS_AnnotationPtr ann, QVariantMap mdata=QVariantMap());

    void setBrushStyle(BrushStyle bstyle) { _style = bstyle; }

    void updateSettings(QVariantMap settings);
    QVariantMap settings();

    void paintGLPostDraw(GUI_Painter* painter, const QVariantMap& mdata);


private:
    void strokeToHard(QPainter* painter, const QPointF& endPoint, QVariantMap mdata=QVariantMap());
    void strokeToSoft(QPainter* painter, const QPointF& endPoint, QVariantMap mdata=QVariantMap());

protected:

    QPointF _lastPoint;
    QColor _penCol;
    QColor _ppenCol;
    int _penWidth;
    QBrush _brush;
    bool _doAntialiasing;

    BrushStyle _style;
    // QPainterPath _path;

};

#endif