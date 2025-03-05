
#include "RlpCore/CNTRL/CommandBase.h"

RLP_SETUP_LOGGER(core, CoreCntrl, CommandFilterBase)
RLP_SETUP_LOGGER(core, CoreCntrl, CommandBase)

const CmdFilterResult CoreCntrl_CommandFilterBase::FilterYes = {true, ""};

CoreCntrl_CommandBase::CoreCntrl_CommandBase(
    QString name,
    QString category
):
    _name(name),
    _category(category)
{
}


QVariant
CoreCntrl_CommandBase::run(QVariant execParams) const
{
    QVariant result;
    return result;
}