//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef EXTREVLENS_RLQTCNTRL_TrackFactoryPlugin_H
#define EXTREVLENS_RLQTCNTRL_TrackFactoryPlugin_H

#include "RlpExtrevlens/RLQTCNTRL/Global.h"

#include "RlpRevlens/DS/TrackFactoryPlugin.h"
#include "RlpExtrevlens/RLQTDS/OnTheGoTrack.h"


class EXTREVLENS_RLQTCNTRL_API RLQTCNTRL_OnTheGoTrackFactoryPlugin : public DS_TrackFactoryPlugin {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    RLQTCNTRL_OnTheGoTrackFactoryPlugin();

    DS_TrackPtr runCreateTrack(DS_Session* session, int trackIdx, QUuid uuid = QUuid::createUuid(), QString owner = "");
    int trackType() { return TRACK_TYPE_ONTHEGO; }

};

#endif
