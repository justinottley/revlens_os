//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_GUI_TL2_MEDIA_TRACK_FACTORY_PLUGIN_H
#define REVLENS_GUI_TL2_MEDIA_TRACK_FACTORY_PLUGIN_H

#include "RlpCore/LOG/Logging.h"

#include "RlpRevlens/DS/Session.h"
#include "RlpRevlens/DS/Node.h"

#include "RlpRevlens/GUI_TL2/Global.h"
#include "RlpRevlens/GUI_TL2/MediaItem.h"
#include "RlpRevlens/GUI_TL2/MediaTrack.h"
#include "RlpRevlens/GUI_TL2/TrackFactoryPluginBase.h"

class GUI_TL2_View;

class REVLENS_GUI_TL2_API GUI_TL2_MediaTrackFactoryPlugin : public GUI_TL2_TrackFactoryPluginBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    GUI_TL2_MediaTrackFactoryPlugin(GUI_TL2_TrackManager* tmgr);
    ~GUI_TL2_MediaTrackFactoryPlugin();

    void startup();

    static GUI_TL2_TrackFactoryPluginBase* create(GUI_TL2_TrackManager* tmgr);

public slots:

    void onSessionChanged(DS_SessionPtr session);
    void onMediaAdded(DS_NodePtr node, QString trackUuidStr, qlonglong frameNum, qlonglong mediaFrameCount);
    void onRemoteCallReceived(QVariantMap callInfo);

    GUI_TL2_Track* runCreateTrack(QString uuidStr, QString trackName);

};

#endif
