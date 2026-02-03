//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef EXTREVLENS_RLANN_BRUSH_TEXT_H
#define EXTREVLENS_RLANN_BRUSH_TEXT_H

#include "RlpExtrevlens/RLANN_BRUSH/Global.h"
#include "RlpExtrevlens/RLANN_BRUSH/TextPreview.h"

#include "RlpExtrevlens/RLANN_DS/BrushBase.h"

#include "RlpGui/WIDGET/TextEdit.h"
#include "RlpGui/WIDGET/IconButton.h"
#include "RlpGui/PANE/PaneFrame.h"

class DISP_GLView;


class RlpAnnBrush_TextAnnotationItem : public QObject {
    Q_OBJECT

signals:
    void textUpdated();

public:
    RLP_DEFINE_LOGGER

    RlpAnnBrush_TextAnnotationItem(
        DISP_GLView* view,
        QString text,
        QPointF pos,
        QColor col,
        int penWidth,
        float imgRatio
    );

    QString text() { return _text; }
    float textSize() { return _tp->textSize(); }
    QPointF pos() { return _tp->pos(); }
    QColor col() { return _col; }
    int penWidth() { return _penWidth; }

    RlpAnnBrush_TextPreview* textPreview() { return _tp; }

public slots:
    void onPreviewEnterPressed(QVariantMap md);

protected:
    QString _text;
    QPointF _pos;
    QColor _col;
    int _penWidth;
    float _imgRatio;

    RlpAnnBrush_TextPreview* _tp;

};


class RlpAnnBrush_TextAnnotation : public QObject {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    enum EditMode {
        EM_VIEW,
        EM_EDIT
    };

    RlpAnnBrush_TextAnnotation(RLANN_DS_AnnotationPtr ann);

    RLANN_DS_BrushBase* clone();

    void setMetadata(QVariantMap mdata) { _mdata = mdata; }
    QVariantMap metadata() { return _mdata; }
    void addTextItem(RlpAnnBrush_TextAnnotationItem* ann);
    void drawAnnotations(EditMode editMode);

public slots:
    void onTextUpdated();

private:
    void drawAnnotation(QPainter* painter, RlpAnnBrush_TextAnnotationItem* ann);

protected:
    RLANN_DS_AnnotationPtr _ann;
    QImage _baseImage;
    QVariantMap _mdata;
    QList<RlpAnnBrush_TextAnnotationItem*> _annotations;

};

class EXTREVLENS_RLANN_BRUSH_API RlpAnnBrush_Text : public RLANN_DS_BrushBase {
    Q_OBJECT

    /*
signals:
    void modeChanged(int mode, qlonglong currFrame);
*/
public:
    RLP_DEFINE_LOGGER

    static const int X_OFFSET;
    static const int Y_OFFSET;

    RlpAnnBrush_Text();
    RLANN_DS_BrushBase* clone();

    void activate(DISP_GLView* view, RLANN_DS_DrawControllerBase* annCntrl);
    void setupFrame(RLANN_DS_AnnotationPtr ann);

    void strokeBegin(QPointF& point, QVariantMap mdata=QVariantMap());
    // void strokeMove(QPointF& point, QVariantMap extra=QVariantMap());
    void strokeTo(RLANN_DS_AnnotationPtr ann, const QPointF& endPoint, QVariantMap mdata=QVariantMap());
    void strokeEnd(RLANN_DS_AnnotationPtr ann, QVariantMap mdata);

    void updateSettings(QVariantMap settings);
    QVariantMap settings();

public slots:

    void onTextUpdated();
    void onTextAnnotationPreviewFocusChanged(RlpAnnBrush_TextPreview* tp, bool focus);

protected:

    QPointF _lastPoint;
    QPointF _pressPos;
    QPointF _pressPosG;

    QColor _penCol;
    int _penWidth;

    QBrush _brush;
    bool _doAntialiasing;
    QVariantMap _mdata;

    RLANN_DS_AnnotationPtr _ann;

    RlpAnnBrush_TextPreview* _tp;
};

#endif