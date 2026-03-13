#ifndef EXTREVLENS_RLANN_GUI_TRACK_ACTIVATE_BUTTON_H
#define EXTREVLENS_RLANN_GUI_TRACK_ACTIVATE_BUTTON_H

#include "RlpExtrevlens/RLANN_GUI/Global.h"

#include "RlpRevlens/GUI_TL2/TrackToggleButton.h"



class RLANN_GUI_Track;

class EXTREVLENS_RLANN_GUI_API RLANN_GUI_TrackActivateButton : public GUI_TL2_TrackToggleButton {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    RLANN_GUI_TrackActivateButton(RLANN_GUI_Track* track);

    void paintOn(GUI_Painter* painter);
    void paintOff(GUI_Painter* painter);

    void mouseReleaseEvent(QMouseEvent* event);

public slots:

    void onActiveTrackChanged(QString trackUuidStr);

private:
    RLANN_GUI_Track* _track;

    QColor _colBgOff;
    QColor _colBgOn;
    QColor _colOff;

    
    QString _imagePathRemote;
    QImage _imageRemote;
};

#endif

