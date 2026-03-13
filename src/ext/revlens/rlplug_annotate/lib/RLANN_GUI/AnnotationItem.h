//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef RLANN_GUI_ANNOTATION_ITEM_H
#define RLANN_GUI_ANNOTATION_ITEM_H

#include "RlpExtrevlens/RLANN_GUI/Global.h"

#include "RlpRevlens/GUI_TL2/Item.h"
#include "RlpRevlens/GUI_TL2/Track.h"
#include "RlpRevlens/GUI_TL2/View.h"

#include "RlpExtrevlens/RLANN_DS/Annotation.h"
#include "RlpExtrevlens/RLANN_GUI/AnnotationHandle.h"

class RLANN_GUI_Track;

class EXTREVLENS_RLANN_GUI_API RLANN_GUI_AnnotationItem : public GUI_TL2_Item {
    Q_OBJECT

public:

    enum MouseMode {
        MM_SCALE,
        MM_POS
    };

    RLP_DEFINE_LOGGER

    RLANN_GUI_AnnotationItem(GUI_TL2_Track* track, RLANN_DS_AnnotationPtr ann, qlonglong startFrame);

    void setYPos(int ypos);

    QRectF boundingRect() const;

    void paint(GUI_Painter* painter);

public slots:

    void onParentSizeChanged(qreal width, qreal height);
    void onStrokeEnd();

    int mouseMode() { return (int)_mouseMode; }

    RLANN_GUI_Track* track();
    RLANN_DS_AnnotationPtr annotation() { return _annotation; }

    void refresh();
    void clear();

private:
    RLANN_DS_AnnotationPtr _annotation;

    MouseMode _mouseMode;

    RLANN_GUI_AnnotationHandle* _handleLeft;
    RLANN_GUI_AnnotationHandle* _handleRight;

    QColor _colPen;
    
    QImage _thumb;
};

#endif
