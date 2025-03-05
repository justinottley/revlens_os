//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include "RlpRevlens/MEDIA/Plugin.h"

#include "RlpExtrevlens/RLANN_DS/FrameBuffer.h"
#include "RlpExtrevlens/RLANN_MEDIA/DrawNode.h"


RLP_SETUP_EXT_LOGGER(RLANN_MEDIA, Node)

RLANN_MEDIA_Node::RLANN_MEDIA_Node(RLANN_DS_DrawControllerBase* drawController, QVariantMap* properties, DS_NodePtr nodeIn):
    DS_Node("RLANN_MEDIA_Node", properties),
    _drawController(drawController)
{
    _inputs.push_back(nodeIn);
}


void
RLANN_MEDIA_Node::readVideo(DS_TimePtr timeInfo, DS_BufferPtr destFrame, bool postUpdate)
{
    // RLP_LOG_DEBUG(timeInfo->videoFrame());

    if (!postUpdate) {

        // if not postUpdate, assume running syncronously in the main thread,
        // reload annotations immediately / syncronously in main thread
        //
        _drawController->scheduleReloadAnnotationTask(timeInfo->videoFrame(), true);
    }

    RLANN_DS_FrameBufferPtr annotationBuf(new RLANN_DS_FrameBuffer(
        destFrame->getOwner(),
        (*destFrame->getBufferMetadata()),
        _drawController));

    QVariantMap* bufMetadata = annotationBuf->getBufferMetadata();
    bufMetadata->insert("draw.frame", (qlonglong)timeInfo->videoFrame());

    destFrame->appendAuxBuffer(annotationBuf);


    _inputs[0]->readVideo(timeInfo, destFrame, postUpdate);
}


void
RLANN_MEDIA_Node::readAudio(DS_TimePtr timeInfo, DS_BufferPtr destBuffer)
{
    _inputs[0]->readAudio(timeInfo, destBuffer);
}


void
RLANN_MEDIA_Node::scheduleReadTask(DS_TimePtr timeInfo,
                                  DS_BufferPtr destFrame,
                                  DS_Node::NodeDataType dataType,
                                  bool optional)
{
    // _inputs[0]->scheduleReadTask(timeInfo, destFrame, dataType, optional);
    
    /*
    std::cout << "RLANN_MEDIA_Node::scheduleReadTask(): " 
              << timeInfo->playlistVideoFrame() 
              << " " << optional << std::endl;
    */
    
    // _drawController->scheduleReloadAnnotationTask(timeInfo->videoFrame());


    QRunnable* task = new RLANN_MEDIA_VideoReadTask(this, timeInfo, destFrame);

    QThreadPool* tpool = _properties->value("video.plugin").
                                      value<MEDIA_Plugin *>()->
                                      getReadThreadPool(dataType);
    
    if ((optional) && (timeInfo->mediaVideoFrame() != 0)) {
        tpool->tryStart(task);
        
    } else {
        tpool->start(task);
    }

}


void
RLANN_MEDIA_Node::scheduleReleaseTask(qlonglong frameNum)
{
    _drawController->scheduleReleaseTask(frameNum);
}


// ------------------------------------------------------------

RLANN_MEDIA_VideoReadTask::RLANN_MEDIA_VideoReadTask(RLANN_MEDIA_Node* node,
                                                     DS_TimePtr timeInfo,
                                                     DS_BufferPtr destFrame) :
    _node(node),
    _timeInfo(timeInfo),
    _destFrame(destFrame)
{
}


void
RLANN_MEDIA_VideoReadTask::run()
{
    _node->readVideo(_timeInfo, _destFrame);
}