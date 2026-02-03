
#include "RlpExtrevlens/RLSGCMD/Commands.h"

#include "RlpCore/PY/Interp.h"


RLP_SETUP_LOGGER(extrevlens, RLSGCMD, SgFindCommand)
RLP_SETUP_LOGGER(extrevlens, RLSGCMD, SgExtensionCommand)

RLSGCMD_SgFindCommand::RLSGCMD_SgFindCommand():
    CoreCntrl_CommandBase("sg.find", "sg")
{
}


QVariant
RLSGCMD_SgFindCommand::run(QVariant execParams) const
{
    QVariantMap msgObj = execParams.toMap();

    LOG_Logging::pprint(msgObj);

    QVariantList args = msgObj.value("args").toList();
    QVariantMap kwargs = msgObj.value("kwargs").toMap();
    QVariant result = PY_Interp::call("rlplug_shotgrid.cmds.sg_find", args, kwargs);

    return result;
}


void
RLSGCMD_SgFindCommand::registerCommand(CoreCntrl_ControllerBase* cntrl)
{
    RLP_LOG_DEBUG(cntrl)
    cntrl->registerCommand(new RLSGCMD_SgFindCommand());
}


// ------


RLSGCMD_SgExtensionCommand::RLSGCMD_SgExtensionCommand():
    CoreCntrl_CommandBase("sg.cmds", "sg")
{
}


QVariant
RLSGCMD_SgExtensionCommand::run(QVariant execParams) const
{
    RLP_LOG_DEBUG(execParams)
    QVariantMap msgObj = execParams.toMap();
    QVariantList args = msgObj.value("args").toList();
    
    QVariant result = PY_Interp::call("rlplug_shotgrid.cmds.run", args);

    return result;
}


void
RLSGCMD_SgExtensionCommand::registerCommand(CoreCntrl_ControllerBase* cntrl)
{
    RLP_LOG_DEBUG(cntrl)
    cntrl->registerCommand(new RLSGCMD_SgExtensionCommand());
}




/*

RLSG_Startup::RLSG_Startup()
{
    RLP_LOG_DEBUG("")
}


void
RLSG_Startup::registerServiceCommands(CoreCntrl_ControllerBase* cntrl)
{
    RLP_LOG_DEBUG(cntrl)
}

*/