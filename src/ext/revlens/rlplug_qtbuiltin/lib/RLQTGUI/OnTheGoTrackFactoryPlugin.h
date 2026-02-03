
#ifndef EXTREVLENS_RLQTGUI_ONTHEGOTRACKFACTORYPLUGIN_H
#define EXTREVLENS_RLQTGUI_ONTHEGOTRACKFACTORYPLUGIN_H

#include "RlpExtrevlens/RLQTGUI/Global.h"

#include "RlpRevlens/GUI_TL2/TrackFactoryPluginBase.h"
#include "RlpRevlens/GUI_TL2/MediaTrack.h"

class EXTREVLENS_RLQTGUI_API RLQTGUI_OnTheGoTrackFactoryPlugin : public GUI_TL2_TrackFactoryPluginBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    RLQTGUI_OnTheGoTrackFactoryPlugin(GUI_TL2_TrackManager* tmgr);

    static GUI_TL2_TrackFactoryPluginBase* create(GUI_TL2_TrackManager* tmgr);

public slots:

    GUI_TL2_Track* runCreateTrack(QString uuidStr, QString trackName);

};

#endif
