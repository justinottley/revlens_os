
#ifndef EXTREVLENS_GUI_TL2_ANNOTATION_HANDLE_H
#define EXTREVLENS_GUI_TL2_ANNOTATION_HANDLE_H

#include "RlpExtrevlens/RLANN_GUI/Global.h"

#include "RlpRevlens/GUI_TL2/View.h"

class RLANN_GUI_AnnotationItem;
class RLANN_GUI_Track;

class EXTREVLENS_RLANN_GUI_API RLANN_GUI_AnnotationHandle : public GUI_ItemBase {
    Q_OBJECT

public:
    enum HandleSide {
        HNDL_NONE,
        HNDL_LEFT,
        HNDL_RIGHT    
    };

public:
    RLP_DEFINE_LOGGER

    RLANN_GUI_AnnotationHandle(HandleSide side, RLANN_GUI_AnnotationItem* item);

    void mouseMoveEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void hoverEnterEvent(QHoverEvent* event);
    void hoverLeaveEvent(QHoverEvent* event);

    QRectF boundingRect() const;
    void paint(GUI_Painter* painter);
    
    GUI_TL2_View* view();
    RLANN_GUI_Track* track();

    void setYPos(int ypos) { setPos(0, ypos); }


private:
    RLANN_GUI_AnnotationItem* _item;
    HandleSide _side;

    bool _inDrag;
    bool _inHandle;
    QPointF _pressPos;

    QColor _colHover;
    QColor _colNormal;
};

#endif
