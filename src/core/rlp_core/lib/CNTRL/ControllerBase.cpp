//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include "RlpCore/CNTRL/ControllerBase.h"

#include "RlpCore/NET/WebSocketServer.h"


RLP_SETUP_LOGGER(core, CoreCntrl, ControllerBase)


CoreCntrl_ControllerBase::CoreCntrl_ControllerBase():
    _wsServer(nullptr)
{
}


void
CoreCntrl_ControllerBase::emitDataReady(QVariantMap info)
{
    emit dataReady(info);
}


void
CoreCntrl_ControllerBase::setWsServer(CoreNet_WebSocketServer* server, bool connectEvents)
{
    RLP_LOG_INFO("")

    _wsServer = server;

    if (!connectEvents)
    {
        RLP_LOG_INFO("Skipping connection of server events to controller")
        return;
    }


    connect(
        _wsServer,
        SIGNAL(msgReceived(QVariantMap)),
        this,
        SLOT(handleRemoteCall(QVariantMap))
    );

    connect(
        _wsServer,
        SIGNAL(serverReady(int)),
        this,
        SLOT(onWsServerReady(int))
    );

    connect(
        this,
        SIGNAL(dataReady(QVariantMap)),
        _wsServer,
        SLOT(sendCallToClient(QVariantMap))
    );

    connect(
        _wsServer,
        SIGNAL(clientDisconnected(QString)),
        this,
        SLOT(onClientDisconnected(QString))
    );
}


void
CoreCntrl_ControllerBase::registerCommand(CoreCntrl_CommandBase* comm, QString name)
{
    if (name == "")
    {
        name = comm->name();
    }

    RLP_LOG_DEBUG(name)

    _commandRegistry.insert(name, comm);
}


void
CoreCntrl_ControllerBase::addCommandFilter(CoreCntrl_CommandFilterBase* commFilter)
{
    RLP_LOG_DEBUG(commFilter->name());

    _commandFilters.push_back(commFilter);
}


CmdFilterResult
CoreCntrl_ControllerBase::checkCommand(QString cmdName, QVariant execParams)
{
    if (_commandRegistry.contains(cmdName))
    {
        CoreCntrl_CommandBase* cmd = _commandRegistry.value(cmdName);

        // RLP_LOG_DEBUG(comm->category() << comm->name())

        for (int i=0; i != _commandFilters.size(); ++i)
        {
            CoreCntrl_CommandFilterBase* cv = _commandFilters.at(i);
            CmdFilterResult res = cv->checkCommand(cmd, execParams);
            if (!res.first)
            {
                QVariantMap noticeInfo;
                noticeInfo.insert("cmd.name", cmd->name());
                noticeInfo.insert("cmd.category", cmd->category());
                noticeInfo.insert("msg", res.second);

                emitNoticeEvent("command_denied", noticeInfo);

                // RLP_LOG_WARN("Filter failed for command" << cmd->name() << "from filter" << cv->name() << ":" << res.second)

                return res;
            }
        }

    } else
    {
        RLP_LOG_WARN("Unknown command:" << cmdName)
    }

    return CoreCntrl_CommandFilterBase::FilterYes;
}


QVariant
CoreCntrl_ControllerBase::runCommand(QString cmdName, QVariant execParams)
{
    QVariant result;

    if (_commandRegistry.contains(cmdName))
    {
        RLP_LOG_DEBUG(cmdName)

        CoreCntrl_CommandBase* cmd = _commandRegistry.value(cmdName);

        for (int i=0; i != _commandFilters.size(); ++i)
        {
            CoreCntrl_CommandFilterBase* cv = _commandFilters.at(i);
            CmdFilterResult res = cv->checkCommand(cmd, execParams);
            if (!res.first)
            {
                QVariantMap noticeInfo;
                noticeInfo.insert("cmd.name", cmd->name());
                noticeInfo.insert("cmd.category", cmd->category());
                noticeInfo.insert("msg", res.second);

                emitNoticeEvent("command_denied", noticeInfo);

                // RLP_LOG_WARN("Filter failed for command" << cmdName << "from filter" << cv->name() << ":" << res.second)

                return result;
            }
        }

        return cmd->run(execParams);
    } else
    {
        RLP_LOG_WARN("Command not found:" << cmdName)
    }

    return QVariant();
}


void
CoreCntrl_ControllerBase::handleRemoteCall(QVariantMap msgObj)
{
    RLP_LOG_WARN("Implement in subclass!")
    RLP_LOG_WARN(msgObj)

}