

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
    
    DS_NodePtr node = _session->getNodeByFrame(frameNum);
    if ((node != nullptr) && (node->getPropertiesPtr()->contains("session.track")))
    {
        QVariantMap* nprops = node->getPropertiesPtr();
        DS_Track* track = nprops->value("session.track").value<DS_Track*>();
        
        bool isNodeEnabled = track->isNodeEnabledByUuid(node->graph()->uuid());
        if (isNodeEnabled)
        {
            return DS_PlaybackMode::getNextFrameNum(frameNum, directionMode, incMode);
        } else
        {
            qlonglong nextFrame = frameNum + nprops->value("media.frame_count").toLongLong();
            // RLP_LOG_DEBUG(frameNum << isNodeEnabled << "checking next frame at" << nextFrame)
            return getNextFrameNum(nextFrame, directionMode, incMode);
        }
    }

    RLP_LOG_WARN(frameNum << "Invalid node / session info, defaulting to base PlaybackMode")

    return DS_PlaybackMode::getNextFrameNum(frameNum, directionMode, incMode);
}