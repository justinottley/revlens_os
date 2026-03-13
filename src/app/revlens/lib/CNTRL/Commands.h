
#ifndef CNTRL_COMMANDS_H
#define CNTRL_COMMANDS_H

#include "RlpRevlens/CNTRL/Global.h"

#include "RlpRevlens/DS/ControllerBase.h"

#include "RlpCore/CNTRL/CommandBase.h"



class CNTRL_RevlensAppCommand : public CoreCntrl_CommandBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    CNTRL_RevlensAppCommand(QString name, QString category, DS_ControllerBase* cntrl):
        CoreCntrl_CommandBase(name, category),
        _cntrl(cntrl)
    {
    }


protected:
    DS_ControllerBase* _cntrl;
};


class CNTRL_TogglePlayStateCommand : public CNTRL_RevlensAppCommand {

public:
    RLP_DEFINE_LOGGER

    CNTRL_TogglePlayStateCommand(DS_ControllerBase* cntrl):
        CNTRL_RevlensAppCommand("TogglePlayState", "playback", cntrl)
    {
    }

    QVariant run(QVariant execParams=QVariant()) const;

};


class CNTRL_ScrubCommand : public CNTRL_RevlensAppCommand {

public:
    RLP_DEFINE_LOGGER

    CNTRL_ScrubCommand(DS_ControllerBase* cntrl):
        CNTRL_RevlensAppCommand("Scrub", "playback", cntrl)
    {
    }

};


class CNTRL_RecenterCommand : public CNTRL_RevlensAppCommand {

public:
    RLP_DEFINE_LOGGER

    CNTRL_RecenterCommand(DS_ControllerBase* cntrl):
        CNTRL_RevlensAppCommand("Recenter", "internal", cntrl)
    {
    }

    QVariant run(QVariant execParams=QVariant()) const;

};

#endif
