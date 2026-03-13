
#ifndef REVLENS_GUI_VIEWER_DROP_OVERLAY_H
#define REVLENS_GUI_VIEWER_DROP_OVERLAY_H

#include "RlpRevlens/GUI/Global.h"

#include "RlpRevlens/DS/Track.h"
#include "RlpRevlens/DS/Session.h"

#include "RlpRevlens/DISP/GLView.h"

class REVLENS_GUI_API GUI_ViewerLoadOverlay : public GUI_ItemBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    GUI_ViewerLoadOverlay(GUI_ItemBase* parent);

    DS_SessionPtr getSession();
    void setSession(DS_SessionPtr session) { _session = session; }

    void setMousePos(QPointF pos);

    void setCompareEnabled(bool cEnabled) { _compareEnabled = cEnabled; }

    int getNumMediaTracks();
    DS_TrackPtr requestedTrack();
    int requestedAction();

    QRectF boundingRect() const;
    void paint(GUI_Painter* painter);

protected:
    DS_SessionPtr _session;

    bool _compareEnabled;

    QPointF _mousePos;
    QImage _loadImgOn;
    QImage _loadImgOff;

    QImage _compareImgOn;
    QImage _compareImgOff;
};

#endif