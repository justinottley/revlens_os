//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include "RlpRevlens/DS/TrackFactoryPlugin.h"


DS_TrackFactoryPlugin::DS_TrackFactoryPlugin(QString name):
    DS_Plugin(name)
{
}


DS_TrackPtr
DS_TrackFactoryPlugin::runCreateTrack(DS_Session* session, int trackIdx, QUuid uuid, QString owner)
{
    return DS_TrackPtr(new DS_Track(session, trackIdx, 0, uuid, owner));
}