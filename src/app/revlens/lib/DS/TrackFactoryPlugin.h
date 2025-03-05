//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_DS_TRACKFACTORY_PLUGIN_H
#define REVLENS_DS_TRACKFACTORY_PLUGIN_H

#include "RlpRevlens/DS/Plugin.h"
#include "RlpRevlens/DS/Track.h"
#include "RlpRevlens/DS/Global.h"

class REVLENS_DS_API DS_TrackFactoryPlugin : public DS_Plugin {
    Q_OBJECT
    
public:
    
    DS_TrackFactoryPlugin(QString name);


public slots:
    
    virtual DS_TrackPtr runCreateTrack(DS_Session* session, int trackIdx, QUuid uuid = QUuid::createUuid(), QString owner = "");
    virtual int trackType() { return TRACK_TYPE_MEDIA; }
};

#endif