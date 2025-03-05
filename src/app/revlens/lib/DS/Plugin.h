//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_DS_PLUGIN_H
#define REVLENS_DS_PLUGIN_H


#include "RlpCore/LOG/Logging.h"

#include "RlpRevlens/DS/Global.h"

class CNTRL_MainController;
class DISP_GLView;

class REVLENS_DS_API DS_Plugin : public QObject {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

public:
    
    DS_Plugin(QString name) :
        _name(name),
        _enabled(false),
        _controller(nullptr),
        _display(nullptr)
    {
    }
    
    DS_Plugin(QString name, bool enabled):
        _name(name),
        _enabled(enabled),
        _controller(nullptr),
        _display(nullptr)
    {
    }

public slots:

    QString name() { return _name; }

    virtual bool setMainController(CNTRL_MainController* controller);
    virtual bool setDisplay(DISP_GLView* display);

    DISP_GLView* display() { return _display; }

    CNTRL_MainController* getMainController() { return _controller; }
    
    virtual bool isEnabled() { return _enabled; }
    virtual void setEnabled(bool enabled);

    void enable() { setEnabled(true); }
    void disable() { setEnabled(false); }

    virtual void handleSyncAction(QString method, QVariantMap kwargs) {}

    // needs to be defined on subclass to get around binding issue
    // this class is not setup for binding due to issues compiling CNTRL and DISP
    // for the MainController and DISP_GLView
    //
    virtual QString getSettingsUI() { return QString(""); }    

protected:

    QString _name;
    bool _enabled;
    CNTRL_MainController* _controller;
    DISP_GLView* _display;

};

Q_DECLARE_METATYPE(DS_Plugin *)


#endif