
#include "RlpRevlens/CNTRL/Commands.h"

#include "RlpRevlens/CNTRL/MainController.h"

RLP_SETUP_LOGGER(revlens, CNTRL, RevlensAppCommand)
RLP_SETUP_LOGGER(revlens, CNTRL, TogglePlayStateCommand)

QVariant
CNTRL_TogglePlayStateCommand::run(QVariant execParams) const
{
    RLP_LOG_DEBUG(execParams)

    CNTRL_MainController::PlaybackState stateEnum = (CNTRL_MainController::PlaybackState)_cntrl->getPlaybackState();
    if (stateEnum == CNTRL_MainController::PLAYING)
    {
        _cntrl->pause();
    }
    else if (stateEnum == CNTRL_MainController::PAUSED)
    {
        _cntrl->play();
    }

    QVariant result(true);
    return result;
}