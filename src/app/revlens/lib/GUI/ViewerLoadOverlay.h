
#ifndef REVLENS_GUI_VIEWER_DROP_OVERLAY_H
#define REVLENS_GUI_VIEWER_DROP_OVERLAY_H

#include "RlpRevlens/GUI/Global.h"

#include "RlpRevlens/DS/Track.h"

#include "RlpRevlens/DISP/GLView.h"

class REVLENS_GUI_API GUI_ViewerLoadOverlay : public GUI_ItemBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    GUI_ViewerLoadOverlay(GUI_ItemBase* parent);

    void setMousePos(QPointF pos);

    int getNumMediaTracks();
    DS_TrackPtr requestedTrack();
    int requestedAction();

    QRectF boundingRect() const;
    void paint(GUI_Painter* painter);

protected:
    QPointF _mousePos;
    QImage _loadImgOn;
    QImage _loadImgOff;

    QImage _compareImgOn;
    QImage _compareImgOff;
};

#endif