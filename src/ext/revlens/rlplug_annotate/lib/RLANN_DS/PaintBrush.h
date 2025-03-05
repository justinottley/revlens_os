//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef EXTREVLENS_RLANN_DS_PAINT_BRUSH_H
#define EXTREVLENS_RLANN_DS_PAINT_BRUSH_H


#include "RlpExtrevlens/RLANN_DS/Global.h"
#include "RlpExtrevlens/RLANN_DS/BrushBase.h"

class EXTREVLENS_RLANN_DS_API RLANN_DS_PaintBrush : public RLANN_DS_BrushBase {

public:
    RLP_DEFINE_LOGGER
    
    
    RLANN_DS_PaintBrush(QString name="BasicPaint");

    void strokeBegin(QPointF& point, QVariantMap settings = QVariantMap());
    void strokeMove(QPointF& point, QVariantMap settings=QVariantMap());
    void strokeTo(QPainter* painter, const QPointF& endPoint);

    void updateSettings(QVariantMap settings);
    QVariantMap settings();

    void paintGLPostDraw(GUI_Painter* painter, const QVariantMap& mdata);


protected:

    QPointF _lastPoint;
    QColor _penCol;
    QColor _ppenCol;
    int _penWidth;
    QBrush _brush;
    bool _doAntialiasing;

};

#endif