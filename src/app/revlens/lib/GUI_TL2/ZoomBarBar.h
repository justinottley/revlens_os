#ifndef REVLENS_GUI_TL2_ZOOMBAR_BAR_H
#define REVLENS_GUI_TL2_ZOOMBAR_BAR_H

#include "RlpRevlens/GUI_TL2/Global.h"


class GUI_TL2_View;
class GUI_TL2_ZoomBar;

class REVLENS_GUI_TL2_API GUI_TL2_ZoomBarBar : public GUI_ItemBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

public:

    GUI_TL2_ZoomBarBar(GUI_ItemBase* zoomBar);

    float xpos() const;

    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);

    virtual QRectF boundingRect() const;
    virtual void paint(GUI_Painter* painter);

public slots:

    void refresh();

    int leftPos() { return _leftPos; }
    void setLeftPos(int pos) { _leftPos = pos; }

    int rightPos() { return _rightPos; }
    void setRightPos(int pos) { _rightPos = pos; }

signals:

    void barMoved(int xDiff);


private:
    GUI_TL2_ZoomBar* _zoomBar;
    bool _inDrag;
    qreal _dragPos;
    qreal _dragPosG;

    int _leftPos;
    int _rightPos;
};


#endif
