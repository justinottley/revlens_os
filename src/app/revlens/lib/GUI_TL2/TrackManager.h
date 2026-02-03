//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_GUI_TL2_TRACKMANAGER_H
#define REVLENS_GUI_TL2_TRACKMANAGER_H

#include "RlpRevlens/GUI_TL2/Global.h"

#include "RlpRevlens/GUI_TL2/Track.h"
#include "RlpRevlens/GUI_TL2/TrackFactoryPluginBase.h"

class GUI_TL2_View;
class GUI_TL2_Controller;

typedef QHash<int, GUI_TL2_Track*>::iterator TrackIterator;
typedef QHash<QString, GUI_TL2_Track*>::iterator TrackUuidIterator;

typedef QMap<QString, GUI_TL2_TrackFactoryPluginBase*>::iterator TrackPluginIterator;

class REVLENS_GUI_TL2_API GUI_TL2_TrackManager : public QObject {
    Q_OBJECT

signals:
    void trackActivated(QString uuidStr);
    void itemSelected(QVariantMap info);
    void trackAdded(QVariantMap info);

public:
    RLP_DEFINE_LOGGER

    GUI_TL2_TrackManager(GUI_TL2_View* view);
    ~GUI_TL2_TrackManager();

    void initPlugins();
    void initCurrSession();

    void onSizeChanged(qreal width, qreal height);

public slots:

    GUI_Scene* scene();
    GUI_TL2_View* view();

    void refresh(qreal height=0);
    void clearTracks();
    bool onTrackDeleted(QString trackUuidStr);

    GUI_TL2_Track* getTrackByUuid(QString uuidStr);
    GUI_TL2_Track* getTrackByIndex(int idx);

    int trackCount() { return _trackMap.size(); }
    QVariantList trackIndexes();


    bool activateTrack(QString uuidStr);
    QList<QString> trackUuidList();

    // QVariant convert(GUI_TL2_Track* track);
    // QVariant convertFromIndex(int i);

public slots:

    void onSessionChanged(DS_SessionPtr session);
    GUI_TL2_Track* onTrackAdded(
        QString uuidStr = QString(),
        QString trackName = QString(),
        QString trackType = QString("Media"),
        bool doRefresh=true
    );

    void onItemSelected(QVariantMap info);


private:

    void updateTrackIndexes();

    GUI_TL2_View* _view;

    QHash<int, GUI_TL2_Track*> _trackMap;
    QHash<QString, GUI_TL2_Track*> _trackUuidMap;
    QMap<QString, GUI_TL2_TrackFactoryPluginBase*> _trackPlugins;

    int _heightTrackSpacing;
    
};

#endif