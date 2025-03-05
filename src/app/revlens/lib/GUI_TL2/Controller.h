//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef CORE_GUI_TIMELINE_CONTROLLER_H
#define CORE_GUI_TIMELINE_CONTROLLER_H

#include "RlpRevlens/GUI_TL2/Global.h"

#include "RlpRevlens/GUI_TL2/View.h"
#include "RlpRevlens/GUI_TL2/TrackManager.h"
#include "RlpRevlens/GUI_TL2/TrackFactoryPluginBase.h"



typedef GUI_TL2_TrackFactoryPluginBase* (*TrackFactoryCreateFunc)(GUI_TL2_TrackManager*);

class REVLENS_GUI_TL2_API GUI_TL2_Controller : public CoreCntrl_ControllerBase {
    Q_OBJECT

signals:
    void activateTrackRequested(QString trackUuidStr);
    void timelineViewCreated(QString uuidStr);

public:
    RLP_DEFINE_LOGGER

    GUI_TL2_Controller(CNTRL_MainController* mc);

    bool registerTrackFactoryPlugin(QString name, TrackFactoryCreateFunc tfunc);

    bool registerTimelineView(QString uuid, GUI_TL2_View* view);

    static GUI_TL2_Controller* instance() { return _instance; }
    static void setInstance(GUI_TL2_Controller* instance) { _instance = instance; }


public slots:

    // void onSessionChanged(DS_SessionPtr session);
    void onActiveTrackChanged(QString uuidStr);


public slots:

    GUI_TL2_TrackFactoryPluginBase* initPlugin(QString name, GUI_TL2_TrackManager* tmgr);

    QStringList pluginNames() { return _trackPlugins.keys(); }
    QStringList viewNames() { return _viewMap.keys(); }

    GUI_TL2_View* getTimelineView(QString name) { return _viewMap.value(name); }
    QList<GUI_TL2_View*> views();

    GUI_TL2_View* getFocusedTimelineView() { return _focusedView; }
    void setFocusedTimelineView(GUI_TL2_View* view) { _focusedView = view; }

    CNTRL_MainController* mainController() { return _mc; }

    void clearViews() { _viewMap.clear(); }


private:

    static GUI_TL2_Controller* _instance;

    QMap<QString, TrackFactoryCreateFunc> _trackPlugins;
    QMap<QString, GUI_TL2_View*> _viewMap;
    GUI_TL2_View* _focusedView;

    CNTRL_MainController* _mc;

};

#endif