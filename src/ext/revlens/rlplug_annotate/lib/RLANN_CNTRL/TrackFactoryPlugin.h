//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef EXTREVLENS_RLANN_CNTRL_TrackFactoryPlugin_H
#define EXTREVLENS_RLANN_CNTRL_TrackFactoryPlugin_H

#include "RlpExtrevlens/RLANN_CNTRL/Global.h"

#include "RlpRevlens/DS/TrackFactoryPlugin.h"

#include "RlpExtrevlens/RLANN_DS/BrushBase.h"
#include "RlpExtrevlens/RLANN_DS/Track.h"

typedef QMap<QString, RLANN_DS_BrushBase*> BrushMap;
typedef QMap<QString, RLANN_DS_BrushBase*>::iterator BrushMapIterator;

class EXTREVLENS_RLANN_CNTRL_API RLANN_CNTRL_TrackFactoryPlugin : public DS_TrackFactoryPlugin {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    RLANN_CNTRL_TrackFactoryPlugin();

    DS_TrackPtr runCreateTrack(DS_Session* session, int trackIdx, QUuid uuid = QUuid::createUuid(), QString owner = "");
    int trackType() { return TRACK_TYPE_ANNOTATION; }

private:
    void initBrushes();


public slots:

    RLANN_DS_BrushBase* getBrush(QString name);
    bool hasBrush(QString name);

    void setBrushSettings(QVariantMap settings);


private:

    BrushMap _brushMap;
    QVariantMap _brushSettings;

};

#endif
