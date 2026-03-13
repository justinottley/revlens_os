

#include "RlpExtrevlens/RLQTGUI/OnTheGoTrackFactoryPlugin.h"


#include "RlpRevlens/GUI_TL2/Track.h"

RLP_SETUP_LOGGER(extrevlens, RLQTGUI, OnTheGoTrackFactoryPlugin)

RLQTGUI_OnTheGoTrackFactoryPlugin::RLQTGUI_OnTheGoTrackFactoryPlugin(GUI_TL2_TrackManager* tmgr):
    GUI_TL2_TrackFactoryPluginBase::GUI_TL2_TrackFactoryPluginBase(QString("OnTheGo"), tmgr)
{
    // startup();
}


GUI_TL2_TrackFactoryPluginBase*
RLQTGUI_OnTheGoTrackFactoryPlugin::create(GUI_TL2_TrackManager* tmgr)
{
    return new RLQTGUI_OnTheGoTrackFactoryPlugin(tmgr);
}



GUI_TL2_Track*
RLQTGUI_OnTheGoTrackFactoryPlugin::runCreateTrack(QString uuidStr, QString trackName)
{
    RLP_LOG_DEBUG(trackName << " " << uuidStr);

    DS_TrackPtr strack = controller()->session()->getTrackByUuid(uuidStr);

    RLP_LOG_DEBUG("Got Session Track")

    GUI_TL2_MediaTrack* track = new GUI_TL2_MediaTrack(_trackManager, strack, trackName);
    track->onVisibilityChanged();

    return track;
}

