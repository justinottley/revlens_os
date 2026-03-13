
#include "RlpExtrevlens/RLANN_GUI/TrackActivateButton.h"
#include "RlpExtrevlens/RLANN_GUI/Track.h"

#include "RlpCore/UTIL/AppGlobals.h"


RLP_SETUP_EXT_LOGGER(RLANN_GUI, TrackActivateButton)

RLANN_GUI_TrackActivateButton::RLANN_GUI_TrackActivateButton(RLANN_GUI_Track* track):
    GUI_TL2_TrackToggleButton::GUI_TL2_TrackToggleButton(track),
    _track(track),
    _colBgOn(QColor(54, 70, 108)),
    _colOff(QColor(50, 50, 50)),
    _colBgOff(QColor(40, 40, 40))
{
    _imagePathRemote = ":wi-fi-signal_gray.png"; 

    // RLP_LOG_DEBUG(_imagePathRemote);

    _imageRemote = QImage(_imagePathRemote).scaledToHeight(size() - 2, Qt::SmoothTransformation);
    
}


void
RLANN_GUI_TrackActivateButton::onActiveTrackChanged(QString trackUuidStr)
{
    // RLP_LOG_DEBUG(trackUuidStr);

    if (trackUuidStr != _track->uuid().toString()) {

        RLP_LOG_WARN("Track UUID DOES NOT MATCH CURRENT TRACK");
        _isToggled = false;

    } else {
        _isToggled = true;
    }

    update();
}


void
RLANN_GUI_TrackActivateButton::paintOn(GUI_Painter* painter)
{

    // RLP_LOG_DEBUG("");
    QColor pcol = _colOff;
    if (_inHover) {
        pcol = _colHoverOn;
    }

    if (_track->isRemote()) {
        painter->drawImage(0, 0, &_imageRemote);

    } else {
        QPen p(pcol);
        p.setWidth(2);
        
        painter->setPen(p);
        
        painter->setBrush(QBrush(_colBgOn));
        painter->drawRect(boundingRect());
        
    }
    
}


void
RLANN_GUI_TrackActivateButton::paintOff(GUI_Painter* painter)
{
    // RLP_LOG_DEBUG("");

    QColor pcol = _colOff;
    if (_inHover) {
        pcol = _colHoverOn;
    }

    QPen p(pcol);
    p.setWidth(2);

    painter->setBrush(QBrush(_colBgOff));
    painter->setPen(p);

    if (_track->isRemote()) {
        painter->drawImage(0, 0, &_imageRemote);

    } else {
        painter->drawRect(boundingRect());
    }
    
}



void
RLANN_GUI_TrackActivateButton::mouseReleaseEvent(QMouseEvent* event)
{
    if ((_isToggled) || (_track->isRemote())) {
        return;
    }

    _isToggled = true;
    emit toggleChanged(_isToggled);
}