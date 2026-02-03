
#ifndef REVLENS_DS_ONTHEGO_TRACK_H
#define REVLENS_DS_ONTHEGO_TRACK_H

#include "RlpExtrevlens/RLQTDS/Global.h"

#include "RlpRevlens/DS/Track.h"

static const int TRACK_TYPE_ONTHEGO = 1;

class REVLENS_DS_API RLQTDS_OnTheGoTrack : public DS_Track {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    RLQTDS_OnTheGoTrack(DS_Session* session, int idx, QUuid uuid, QString owner);

public slots:
    void onNoticeEvent(QString evtName, QVariantMap evtInfo);


private:
    static DS_Track* _INSTANCE;


};

#endif