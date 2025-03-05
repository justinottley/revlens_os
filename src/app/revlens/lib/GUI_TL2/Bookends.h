#ifndef REVLENS_GUI_TL2_BOOKENDS_H
#define REVLENS_GUI_TL2_BOOKENDS_H

#include "RlpRevlens/GUI_TL2/Global.h"
#include "RlpRevlens/GUI_TL2/BookendDragButton.h"

class GUI_TL2_View;

class REVLENS_GUI_TL2_API GUI_TL2_Bookends : public GUI_ItemBase {
    Q_OBJECT

public:

    RLP_DEFINE_LOGGER

    GUI_TL2_Bookends(GUI_TL2_View* view);

    void onParentSizeChanged(qreal nwidth, qreal nheight);
    
    virtual QRectF boundingRect() const;
    virtual void paint(GUI_Painter* painter);

    GUI_TL2_View* view() { return _view; }

    int buttonWidth() { return _btnDragLeft->width(); }
    void refresh();
    

public slots:

    qlonglong inFrame() { return _btnDragLeft->posFrame(); }
    void setInFrame(qlonglong frame) { _btnDragLeft->setPosFrame(frame); }

    qlonglong outFrame() { return _btnDragRight->posFrame(); }
    void setOutFrame(qlonglong frame) { _btnDragRight->setPosFrame(frame); }

    void onZoomChanged(); // float zoom, int side, int leftXPos, int rightXPos);
    void onPanChanged();
    void onButtonFrameChanged(int side, qlonglong frame);
    
    GUI_TL2_BookendDragButton* btnLeft() { return _btnDragLeft; }
    GUI_TL2_BookendDragButton* btnRight() { return _btnDragRight; }

signals:

    void inFrameChanged(qlonglong frame);
    void outFrameChanged(qlonglong frame);

private:

    GUI_TL2_View* _view;

    GUI_TL2_BookendDragButton* _btnDragLeft;
    GUI_TL2_BookendDragButton* _btnDragRight;

};

#endif

