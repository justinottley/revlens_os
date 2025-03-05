
#ifndef rlplug_QTBUILTIN_DISP_COMPOSITE_WIPE_SPLITTER_H
#define rlplug_QTBUILTIN_DISP_COMPOSITE_WIPE_SPLITTER_H


#include "RlpCore/LOG/Logging.h"

#include "RlpGui/BASE/Scene.h"

#include "RlpRevlens/DISP/GLView.h"

#include "RlpExtrevlens/RLQTDISP/Global.h"


class EXTREVLENS_RLQTDISP_API RLQTDISP_CompositeWipeSplitter : public GUI_ItemBase {
    Q_OBJECT

signals:
    void posChanged(float xpos);

public:
    RLP_DEFINE_LOGGER

    RLQTDISP_CompositeWipeSplitter(DISP_GLView* parent);

    void setEnabled(bool enabled) { _enabled = enabled; }

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);

    void hoverEnterEvent(QHoverEvent* event);
    void hoverLeaveEvent(QHoverEvent* event);

    void onParentSizeChanged(qreal nwidth, qreal nheight);

    QRectF boundingRect() const;
    void paint(GUI_Painter* painter);

public slots:

    float posPercent() { return _posPercentX; }
    void setPosPercent(float percent) { _posPercentX = percent; }

protected:

    DISP_GLView* _display;

    bool _enabled;
    bool _inHover;
    bool _inDrag;

    qreal _dragPos;
    qreal _dragPosG;
    float _posPercentX;

};


#endif