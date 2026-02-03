
#ifndef EXTREVLENS_RLSGCMD_COMMANDS_H
#define EXTREVLENS_RLSGCMD_COMMANDS_H

#include "RlpExtrevlens/RLSGCMD/Global.h"

#include "RlpCore/CNTRL/CommandBase.h"
#include "RlpCore/CNTRL/ControllerBase.h"


class RLSGCMD_SgFindCommand : public CoreCntrl_CommandBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    RLSGCMD_SgFindCommand();

    QVariant run(QVariant execParams) const;

public slots:

    static RLSGCMD_SgFindCommand* new_RLSGCMD_SgFindCommand()
    {
        return new RLSGCMD_SgFindCommand();
    }

    void registerCommand(CoreCntrl_ControllerBase* cntrl);

};

class RLSGCMD_SgExtensionCommand : public CoreCntrl_CommandBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER
    
    RLSGCMD_SgExtensionCommand();

    QVariant run(QVariant execParams) const;

public slots:

    static RLSGCMD_SgExtensionCommand* new_RLSGCMD_SgExtensionCommand()
    {
        return new RLSGCMD_SgExtensionCommand();
    }

    void registerCommand(CoreCntrl_ControllerBase* cntrl);


};

/*

class RLSG_Startup : public QObject {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    RLSG_Startup();

public slots:

    void registerServiceCommands(CoreCntrl_ControllerBase* cntrl);

};
*/

#endif
