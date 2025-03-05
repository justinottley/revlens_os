
#ifndef EXTREVLENS_RLANN_GUI_ANNOTATION_TRACK_FACTORY_PLUGIN_H
#define EXTREVLENS_RLANN_GUI_ANNOTATION_TRACK_FACTORY_PLUGIN_H

#include "RlpExtrevlens/RLANN_GUI/Global.h"

#include "RlpRevlens/DS/Session.h"
#include "RlpRevlens/CNTRL/MainController.h"

#include "RlpExtrevlens/RLANN_CNTRL/DrawController.h"
#include "RlpExtrevlens/RLANN_GUI/Track.h"

class EXTREVLENS_RLANN_GUI_API RLANN_GUI_AnnotationTrackFactoryPlugin : public GUI_TL2_TrackFactoryPluginBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    RLANN_GUI_AnnotationTrackFactoryPlugin(
        GUI_TL2_TrackManager* tmgr,
        RLANN_CNTRL_DrawController* drawController);

    void initConnections();
    void postCreateTrack(GUI_TL2_Track* track);

    static GUI_TL2_TrackFactoryPluginBase* create(GUI_TL2_TrackManager* tmgr);

public slots:

    GUI_TL2_Track* runCreateTrack(QString uuidStr, QString trackName);

    void onSessionChanged(DS_SessionPtr session);
    void onDrawStart(qlonglong drawFrame, QString trackUuidStr);
    void onHoldFrame(QString trackUuidStr, QString annUuidStr, qlonglong srcFrame, qlonglong destFrame);
    void onReleaseFrame(QString trackUuidStr, QString annUuidStr, qlonglong frame);
    void onTrackDataChanged(QString trackUuidStr, QVariantMap trackData);
    void onSyncAction(QString action, QVariantMap kwargs);

private:

    RLANN_CNTRL_DrawController* _drawController;
};

#endif
