//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef EXTREVLENS_RLQTDISP_PLUGIN_H
#define EXTREVLENS_RLQTDISP_PLUGIN_H

#include "RlpExtrevlens/RLQTDISP/Global.h"
#include "RlpRevlens/DS/EventPlugin.h"


class EXTREVLENS_RLQTDISP_API RLQTDISP_HUD : public DS_EventPlugin {
    Q_OBJECT
    
public:
    RLP_DEFINE_LOGGER
    
    RLQTDISP_HUD();

    DS_EventPlugin* duplicate();

    void reset();

    bool setMainController(DS_ControllerBase* controller);

    bool mousePressEventInternal(QMouseEvent* event);
    bool mouseMoveEventInternal(QMouseEvent* event);
    bool mouseReleaseEventInternal(QMouseEvent* event);
    
    void paintGLPostDraw(GUI_Painter* painter, const QVariantMap& metadata);

public slots:

    virtual void onDisplayDeregistered(DISP_GLView* display);

public slots:

    void setHudConfig(QVariantMap hudConfig);

    QVariantMap getHudConfig() { return _hudConfig; }
    QVariantMap getHudInfo(QString hudName);
    bool hasHud(QString displayUuid, QString hudName);
    void setHudVisible(QString displayUuid, QString hudName, bool isVisible);
    bool isHudVisible(QString displayUuid, QString hudName);

    bool addHudConfig(QString hudName, QVariantMap hudConfig);

    QString getHudItemString(QVariantMap hudItem, const QVariantMap metadata);

private:

    QMutex* _lock;
    QVariantMap _hudConfig;

    QPointF _pressPos;
    bool _inPress;

    // Not ideal..
    QString _currHud;

    QString _lastNodeUuid;
    bool _forceReload;
};



Q_DECLARE_METATYPE(RLQTDISP_HUD *)

#endif

