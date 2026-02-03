
#ifndef REVLENS_GUI_VIEW_LOAD_OVERLAY_H
#define REVLENS_GUI_VIEW_LOAD_OVERLAY_H

#include "RlpRevlens/GUI_VIEW/Global.h"

#include "RlpRevlens/DS/Track.h"

#include "RlpRevlens/DISP/GLView.h"

class REVLENS_GUI_VIEW_API GUI_VIEW_LoadOverlay : public GUI_ItemBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    GUI_VIEW_LoadOverlay(GUI_ItemBase* parent);

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