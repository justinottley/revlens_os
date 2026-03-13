

#include "RlpRevlens/DS/PlaybackModeSession.h"

#include "RlpRevlens/DS/Session.h"

RLP_SETUP_LOGGER(revlens, DS, SessionPlaybackMode)

DS_SessionPlaybackMode::DS_SessionPlaybackMode():
    DS_PlaybackMode()
{
}


DS_PlaybackMode::FrameInfo
DS_SessionPlaybackMode::getNextFrameNum(
    qlonglong frameNum,
    DS_PlaybackMode::DirectionMode directionMode,
    DS_PlaybackMode::IncMode incMode)
{
    // RLP_LOG_DEBUG(frameNum)

    if (_session == nullptr)
    {
        return DS_PlaybackMode::getNextFrameNum(frameNum, directionMode, incMode);
    }


    DS_PlaybackMode::FrameInfo fi = DS_PlaybackMode::getNextFrameNum(frameNum, directionMode, incMode);
    qlonglong nextFrameNum = fi.first;

    DS_NodePtr node = _session->getNodeByFrame(nextFrameNum);
    if ((node != nullptr) && (node->getPropertiesPtr()->contains("session.track")))
    {
        QVariantMap* nprops = node->getPropertiesPtr();
        DS_Track* track = nprops->value("session.track").value<DS_Track*>();
        
        bool isNodeEnabled = track->isNodeEnabledByUuid(node->graph()->uuid());

        if (isNodeEnabled)
        {
            return fi;
        } else
        {
            qlonglong nextPotentialFrame = frameNum + nprops->value("media.frame_count").toLongLong();
            return getNextFrameNum(nextPotentialFrame, directionMode, incMode);
        }

    }

    RLP_LOG_WARN(frameNum << "Invalid node / session info, defaulting to base PlaybackMode")
    return DS_PlaybackMode::getNextFrameNum(frameNum, directionMode, incMode);
}