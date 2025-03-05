//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef EXTREVLENS_RLANN_MP_BRUSH_H
#define EXTREVLENS_RLANN_MP_BRUSH_H

#include "RlpExtrevlens/RLANN_MP/Global.h"
#include "RlpExtrevlens/RLANN_DS/BrushBase.h"

#include "RlpExtrevlens/RLANN_DS/Annotation.h"

#include "mypaint-brush.h"

class RLANN_MP_Surface;

class EXTREVLENS_RLANN_MP_API RLANN_MP_Brush : public RLANN_DS_BrushBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    RLANN_MP_Brush(QString name="MyPaint");

    void activate(DISP_GLView* view, RLANN_DS_DrawControllerBase* annCntrl);

    void strokeBegin(QPointF& point, QVariantMap settings = QVariantMap());
    void strokeTo(QPainter* painter, const QPointF& endPoint);

    void setupFrame(RLANN_DS_AnnotationPtr ann);
    void updateSettings(QVariantMap settings);
    QVariantMap settings();


public slots:
    void loadBrush(QByteArray data);
    void setBrushByName(QString brushCat, QString brushName);
    void setBrushByPath(QString brushPath);

    void onBrushRequested(QVariantMap brushInfo);

private:
    void initBrush();

    void setColor(QColor newColor);

    float getValue(MyPaintBrushSetting setting);
    void setValue(MyPaintBrushSetting setting, float value);

protected:

    QPointF _lastPoint;
    QColor _penCol;
    int _penWidth;

    bool _doAntialiasing;

    MyPaintBrush* _brush;
    QString _brushCategory;
    QString _brushName;

    QHash<QUuid, RLANN_MP_Surface*> _surfMap;
    QUuid _currUuid;

};

#endif