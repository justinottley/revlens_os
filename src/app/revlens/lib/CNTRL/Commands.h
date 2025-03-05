
#ifndef CNTRL_COMMANDS_H
#define CNTRL_COMMANDS_H

#include "RlpRevlens/CNTRL/Global.h"

#include "RlpCore/CNTRL/CommandBase.h"

class CNTRL_MainController;

class CNTRL_RevlensAppCommand : public CoreCntrl_CommandBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    CNTRL_RevlensAppCommand(QString name, QString category, CNTRL_MainController* cntrl):
        CoreCntrl_CommandBase(name, category),
        _cntrl(cntrl)
    {
    }


protected:
    CNTRL_MainController* _cntrl;
};


class CNTRL_TogglePlayStateCommand : public CNTRL_RevlensAppCommand {

public:
    RLP_DEFINE_LOGGER

    CNTRL_TogglePlayStateCommand(CNTRL_MainController* cntrl):
        CNTRL_RevlensAppCommand("TogglePlayState", "playback", cntrl)
    {
    }

     QVariant run(QVariant execParams=QVariant()) const;

};


class CNTRL_ScrubCommand : public CNTRL_RevlensAppCommand {

public:
    RLP_DEFINE_LOGGER

    CNTRL_ScrubCommand(CNTRL_MainController* cntrl):
        CNTRL_RevlensAppCommand("Scrub", "playback", cntrl)
    {
    }

};

#endif
