
#include "RlpExtrevlens/RLANN_MEDIA/StreamingDrawNode.h"

RLP_SETUP_LOGGER(extrevlens, RLANN_MEDIA, StreamingNode)

RLANN_MEDIA_StreamingNode::RLANN_MEDIA_StreamingNode(
    RLANN_DS_DrawControllerBase* drawController,
    QVariantMap* properties,
    DS_NodePtr nodeIn):
        DS_Node("RLANN_MEDIA_StreamingNode", properties),
        _drawController(drawController),
        _fbuf(nullptr)
{
    _inputs.push_back(nodeIn);
}


void
RLANN_MEDIA_StreamingNode::readVideo(DS_TimePtr timeInfo, DS_BufferPtr destFrame, bool postUpdate)
{
    // RLP_LOG_DEBUG(timeInfo->videoFrame());

    // if (_fbuf == nullptr)
    // {
    //     RLP_LOG_DEBUG("Setting up draw framebuffer")

    //     _fbuf = RLANN_DS_FrameBufferPtr(new RLANN_DS_FrameBuffer(
    //         destFrame->getOwner(),
    //         (*destFrame->getBufferMetadata()),
    //         _drawController)
    //     );
    // }

    // QVariantMap* bufMetadata = _fbuf->getBufferMetadata();
    // bufMetadata->insert("draw.frame", (qlonglong)timeInfo->videoFrame());

    // destFrame->appendAuxBuffer(_fbuf);


    _inputs[0]->readVideo(timeInfo, destFrame, postUpdate);
}



// ------------------------------------------------------------



RLANN_MEDIA_StreamingVideoReadTask::RLANN_MEDIA_StreamingVideoReadTask(
    RLANN_MEDIA_StreamingNode* node,
    DS_TimePtr timeInfo,
    DS_BufferPtr destFrame):
        _node(node),
        _timeInfo(timeInfo),
        _destFrame(destFrame)
{
}


void
RLANN_MEDIA_StreamingVideoReadTask::run()
{
    _node->readVideo(_timeInfo, _destFrame);
}


