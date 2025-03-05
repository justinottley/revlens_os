
#ifndef EXTREVLENS_RLANN_GUI_TRACK_H
#define EXTREVLENS_RLANN_GUI_TRACK_H

#include "RlpExtrevlens/RLANN_GUI/Global.h"

#include "RlpRevlens/GUI_TL2/Track.h"
#include "RlpRevlens/GUI_TL2/TrackManager.h"

#include "RlpExtrevlens/RLANN_DS/Track.h"
#include "RlpExtrevlens/RLANN_CNTRL/DrawController.h"



// typedef QMap<qlonglong, RLANN_GUI_AnnotationItem*> AnnotationItemMap;
// typedef QMap<qlonglong, bool> AnnotationItemMap;

class EXTREVLENS_RLANN_GUI_API RLANN_GUI_Track : public GUI_TL2_Track {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    ~RLANN_GUI_Track();

    RLANN_GUI_Track(
        GUI_TL2_TrackManager* trackManager,
        DS_TrackPtr strack,
        QString trackName,
        RLANN_CNTRL_DrawController* drawController
    );

    void initCurrSession();
    void initButtons();
    void initGearMenu();
    void initTrackConnections();

    RLANN_DS_Track* strack();
    void addAnnotationItem(RLANN_DS_AnnotationPtr ann, qlonglong frame);

    RLANN_CNTRL_DrawController* drawController() { return _drawController; }

    qreal trackHeight();

public slots:

    QString label();

    void onVisibilityChanged();
    void onActiveTrackToggled(bool vis); // from activate toggle button
    void onPrevItemClicked();
    void onNextItemClicked();
    void onExportAnnotationsRequested();
    void onNewDrawingRequested();
    void onGhostingChanged(QVariantMap val);

    bool hasAnnotationItem(qlonglong frame);
    void holdFrame(qlonglong srcFrame, qlonglong destFrame);
    void releaseFrame(qlonglong destFrame);
    bool clearAnnotation(QString annUuidStr);

private:

    RLANN_CNTRL_DrawController* _drawController;

};

#endif