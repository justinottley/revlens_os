//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_GUI_TL2_TRACK_FACTORY_PLUGIN_H
#define REVLENS_GUI_TL2_TRACK_FACTORY_PLUGIN_H

#include "RlpRevlens/GUI_TL2/Global.h"

#include "RlpRevlens/DS/Session.h"
#include "RlpRevlens/CNTRL/MainController.h"
#include "RlpRevlens/GUI_TL2/Track.h"

class GUI_TL2_TrackManager;
class GUI_TL2_View;

class REVLENS_GUI_TL2_API GUI_TL2_TrackFactoryPluginBase : public QObject {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    GUI_TL2_TrackFactoryPluginBase(QString name, GUI_TL2_TrackManager* tmgr);
    virtual ~GUI_TL2_TrackFactoryPluginBase();

    virtual void startup();

    virtual void onSessionChanged(DS_SessionPtr session) {}
    virtual GUI_TL2_Track* runCreateTrack(QString uuidStr = QString(), QString trackName = QString()) {
        return nullptr;
    }

    virtual void postCreateTrack(GUI_TL2_Track* track) {}
    // virtual QVariant convert(GUI_TL2_Track* track) { return QVariant(); }

    CNTRL_MainController* controller();

public slots:

    QString name() { return _name; }

    GUI_TL2_TrackManager* trackManager() { return _trackManager; }

    QVariant getTrack() { return QVariant(); }

    static GUI_TL2_TrackFactoryPluginBase* create(GUI_TL2_TrackManager* tmgr) { return nullptr; }

protected:
    QString _name;
    GUI_TL2_TrackManager* _trackManager;

};

#endif
