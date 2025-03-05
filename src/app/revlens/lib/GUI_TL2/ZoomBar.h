#ifndef REVLENS_GUI_TL2_ZOOMBAR_H
#define REVLENS_GUI_TL2_ZOOMBAR_H

#include "RlpRevlens/GUI_TL2/Global.h"

#include "RlpRevlens/GUI_TL2/ZoomBarBar.h"
#include "RlpRevlens/GUI_TL2/ZoomBarDragButton.h"


class REVLENS_GUI_TL2_API GUI_TL2_ZoomBar : public GUI_ItemBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    GUI_TL2_ZoomBar(GUI_ItemBase* view);

    // QPointF pos() const;

public slots:

    GUI_TL2_View* view() { return _view; }

    GUI_TL2_ZoomBarDragButton* btnDragLeft() { return _btnDragLeft; }
    GUI_TL2_ZoomBarDragButton* btnDragRight() { return _btnDragRight; }
    GUI_TL2_ZoomBarBar* bar() { return _bar; }

    void refresh();

    // void resizeEvent(QResizeEvent* event);
    void onParentSizeChanged(qreal width, qreal height);

    virtual QRectF boundingRect() const;
    virtual void paint(GUI_Painter* painter);

signals:

    void zoomChanged(float zoom, int side, int leftXPos, int rightXPos);
    void panChanged(int side, int xDiff);


public slots:

    void onButtonMoved(int side, float xpos);
    void onBarMoved(int xDiff);

    QColor colBackground() { return QColor(35, 35, 35); }
    QColor colForeground() { return QColor(65, 65, 65); }
    QColor colHighlight() { return QColor(45, 45, 200); }

private:

    GUI_TL2_View* _view;

    GUI_TL2_ZoomBarBar* _bar;
    GUI_TL2_ZoomBarDragButton* _btnDragLeft;
    GUI_TL2_ZoomBarDragButton* _btnDragRight;

};

#endif