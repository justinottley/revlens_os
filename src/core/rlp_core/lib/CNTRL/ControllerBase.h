//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef CORE_CNTRL_CONTROLLERBASE_H
#define CORE_CNTRL_CONTROLLERBASE_H

#include "RlpCore/CNTRL/Global.h"
#include "RlpCore/CNTRL/CommandBase.h"

#include "RlpCore/NET/WebSocketServer.h"

class CORE_CNTRL_API CoreCntrl_ControllerBase : public QObject {
    Q_OBJECT

signals:

    void dataReady(QVariantMap info);
    void logRecordReady(QVariantMap info);


public:
    RLP_DEFINE_LOGGER

    CoreCntrl_ControllerBase();
    virtual ~CoreCntrl_ControllerBase() {}

public slots:

    // virtual void sendCall(QString method, QVariantList args, QVariantMap kwargs) {}
    // virtual void emitSyncAction(QString action, QVariantMap& kwargs) {}

    void emitDataReady(QVariantMap info);
    virtual void emitNoticeEvent(QString noticeName, QVariantMap noticeInfo) {}

    void setWsServer(CoreNet_WebSocketServer* server, bool connectEvents=true);
    CoreNet_WebSocketServer* wsServer() { return _wsServer; }

    virtual void onWsServerReady(int port) {}
    virtual void onClientDisconnected(QString ident) {}
    virtual void handleRemoteCall(QVariantMap msgObj);

    void registerCommand(CoreCntrl_CommandBase* comm, QString name="");
    void addCommandFilter(CoreCntrl_CommandFilterBase* commFilter);
    QVariant runCommand(QString cmdName, QVariant execParams=QVariant());
    CmdFilterResult checkCommand(QString cmdName, QVariant execParams=QVariant());


protected:
    CoreNet_WebSocketServer* _wsServer;

    QMap<QString, CoreCntrl_CommandBase*> _commandRegistry;
    QList<CoreCntrl_CommandFilterBase*> _commandFilters;

};

typedef std::shared_ptr<CoreCntrl_ControllerBase> CoreDs_ControllerPtr;


Q_DECLARE_METATYPE(CoreDs_ControllerPtr)

#endif
