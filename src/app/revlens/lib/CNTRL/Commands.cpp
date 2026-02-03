
#include "RlpRevlens/CNTRL/Commands.h"

#include "RlpRevlens/CNTRL/MainController.h"

RLP_SETUP_LOGGER(revlens, CNTRL, RevlensAppCommand)
RLP_SETUP_LOGGER(revlens, CNTRL, TogglePlayStateCommand)
RLP_SETUP_LOGGER(revlens, CNTRL, RecenterCommand)

QVariant
CNTRL_TogglePlayStateCommand::run(QVariant execParams) const
{
    // RLP_LOG_DEBUG(_cntrl << _cntrl->getPlaybackState())

    CNTRL_MainController::PlaybackState stateEnum = (CNTRL_MainController::PlaybackState)_cntrl->getPlaybackState();
    if (stateEnum == CNTRL_MainController::PLAYSTATE_PLAYING)
    {
        _cntrl->pause();
    }
    else if (stateEnum == CNTRL_MainController::PLAYSTATE_PAUSED)
    {
        _cntrl->play();
    }

    QVariant result(true);
    return result;
}


QVariant
CNTRL_RecenterCommand::run(QVariant execParams) const
{
    RLP_LOG_DEBUG(execParams)

    QVariantList args = execParams.toList();
    if (args.size() == 0)
    {
        RLP_LOG_ERROR("invalid args")
        return QVariant(false);
    }
    qlonglong frame = args.at(0).toLongLong();

    if (frame == 0)
    {
        RLP_LOG_ERROR("invalid frame")
        return QVariant(false);
    }

    _cntrl->gotoFrame(frame, /*recenter=*/ true, /*schedule=*/ true, /*emitSync=*/ false);

    return QVariant(true);

}