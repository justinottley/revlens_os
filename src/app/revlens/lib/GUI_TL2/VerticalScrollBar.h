#ifndef REVLENS_GUI_TL2_VERTICAL_SCROLLBAR_H
#define REVLENS_GUI_TL2_VERTICAL_SCROLLBAR_H

#include "RlpRevlens/GUI_TL2/Global.h"


class GUI_TL2_View;
class GUI_TL2_VerticalScrollBar;

class REVLENS_GUI_TL2_API GUI_TL2_VerticalScrollBarHandle : public GUI_ItemBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER
    
    GUI_TL2_VerticalScrollBarHandle(GUI_TL2_VerticalScrollBar* vbar);

    float posPercent() { return _posPercent; }

    void hoverEnterEvent(QHoverEvent* event);
    void hoverLeaveEvent(QHoverEvent* event);

    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);

    QRectF boundingRect() const;
    void paint(GUI_Painter* painter);

    void refresh();


signals:
    void handlePosChanged(float posPercent);

private:
    GUI_TL2_VerticalScrollBar* _vbar;
    bool _hover;

    qreal _dragPos;
    qreal _dragPosG;

    float _posPercent;
};

class REVLENS_GUI_TL2_API GUI_TL2_VerticalScrollBar : public GUI_ItemBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    static const int AREA_WIDTH;

    GUI_TL2_VerticalScrollBar(GUI_TL2_View* view);

    void onParentSizeChanged(qreal width, qreal height);
    void updatePos();
    
    QRectF boundingRect() const;
    void paint(GUI_Painter* painter);

    QColor colBackground() { return _colBackground; }
    QColor colForeground() { return _colForeground ; }
    QColor colHighlight() { return _colHighlight; }

    GUI_TL2_View* view() { return _view; }
    GUI_TL2_VerticalScrollBarHandle* handle() { return _handle; }

    void refresh() { _handle->refresh(); }

public slots:
    void onHandlePosChanged(float pos);

signals:
    void scrollPosChanged(float pos);

private:
    GUI_TL2_View* _view;
    GUI_TL2_VerticalScrollBarHandle* _handle;

    QColor _colBackground;
    QColor _colForeground;
    QColor _colHighlight;

};

#endif
