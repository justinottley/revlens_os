
#include "RlpExtrevlens/RLQTCNTRL/OnTheGoTrackFactoryPlugin.h"


RLP_SETUP_LOGGER(extrevlens, RLQTCNTRL, OnTheGoTrackFactoryPlugin)

RLQTCNTRL_OnTheGoTrackFactoryPlugin::RLQTCNTRL_OnTheGoTrackFactoryPlugin():
    DS_TrackFactoryPlugin("OnTheGo")
{
}


DS_TrackPtr
RLQTCNTRL_OnTheGoTrackFactoryPlugin::runCreateTrack(DS_Session* session, int trackIdx, QUuid uuid, QString owner)
{
    RLP_LOG_DEBUG(trackIdx);

    RLQTDS_OnTheGoTrack* track = new RLQTDS_OnTheGoTrack(session, trackIdx, uuid, owner);

    return DS_TrackPtr(track);
}