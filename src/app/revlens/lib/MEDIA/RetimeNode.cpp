

#include "RlpRevlens/MEDIA/Plugin.h"
#include "RlpRevlens/MEDIA/RetimeNode.h"

RLP_SETUP_LOGGER(revlens, MEDIA, RetimeNode)

MEDIA_RetimeNode::MEDIA_RetimeNode(QVariantMap* properties, DS_NodePtr nodeIn):
    DS_Node("MEDIA_Retime", properties),
    _enabled(true),
    _frameIn(0),
    _frameOut(0)
{
    _inputs.clear();
    _inputs.push_back(nodeIn);
    
    if (_enabled) {
        qlonglong inFrameCount = _properties->value("media.frame_count").toLongLong();
        _frameIn = _properties->value("media.frame_in").toLongLong();
        _frameOut = _properties->value("media.frame_out").toLongLong();

        // number of frames from the source clip that have been cut out from head and tail
        if ((_frameIn != 0) && (_frameOut != 0)) {
            qlonglong totalFramesRemoved = _frameIn + _frameOut; // (inFrameCount - _frameOut);
            qlonglong cutFrameLength = inFrameCount - totalFramesRemoved;

            RLP_LOG_DEBUG("in frame count: " << inFrameCount
                << " frame_in: " << _frameIn << " frame_out: " << _frameOut
                << " totalCutFrames: " << totalFramesRemoved << " cutFrameLength: " << cutFrameLength);

            _properties->insert("media.frame_count", (qlonglong)(cutFrameLength));
        }
        
    }
    
}


void
MEDIA_RetimeNode::retime(DS_TimePtr timeInfo)
{
    if (!_enabled) {
        return;
    }
    //return;
    
    // RLP_LOG_DEBUG("frameIn: " << _frameIn << " " << timeInfo->videoFrame() << " -> " << timeInfo->videoFrame() + _frameIn);
    
    // Retime in place
    timeInfo->setVideoFrame(timeInfo->videoFrame() + _frameIn);
}


void
MEDIA_RetimeNode::readVideo(DS_TimePtr timeInfo, DS_BufferPtr destFrame, bool postUpdate)
{
    // std::cout << "MEDIA_RetimeNode::readVideo(): " << timeInfo->mediaVideoFrame();
    
    // Override owner of buffer so when event is posted after read we can intercept,
    // remap back to source space, and then resend event to source owner
    //
    QVariant ownerWrapper;
    ownerWrapper.setValue(destFrame->getOwner());
    
    destFrame->getBufferMetadata()->insert("node.retime.buffer_src_owner", ownerWrapper);
    destFrame->setOwner(this);
    
    retime(timeInfo);
    
    // std::cout << " retimed frame: " << timeInfo->mediaVideoFrame() << std::endl;
    
    return _inputs[0]->readVideo(timeInfo, destFrame, postUpdate);
}


void
MEDIA_RetimeNode::readAudio(DS_TimePtr timeInfo, DS_BufferPtr destBuffer)
{
    // std::cout << "MEDIA_RetimeNode::readAudio()" << std::endl;
    
    retime(timeInfo);
    return _inputs[0]->readAudio(timeInfo, destBuffer);
}


void
MEDIA_RetimeNode::scheduleReadTask(DS_TimePtr timeInfo,
                                   DS_BufferPtr destFrame,
                                   DS_Node::NodeDataType dataType,
                                   bool optional)
{
    QRunnable* task = new MEDIA_RetimeVideoReadTask(this, timeInfo, destFrame);
    
    QThreadPool* tpool = _properties->value("video.plugin").
                                           value<MEDIA_Plugin *>()->
                                           getReadThreadPool(dataType);
    
    // STARTUP HACK: add an exception for frame 0 as a fix for getting the
    // display to show picture at startup.. not a great solution, especially
    // if the user requests a start frame other than 0 for startup...
    //
    
    if ((optional) && (timeInfo->mediaVideoFrame() != 0)) {        
        tpool->tryStart(task);
    } else {
        tpool->start(task);
    }
    
}


bool
MEDIA_RetimeNode::event(QEvent* event)
{
    
    if (event->type() == MEDIA_FrameReadEventType) {
        
        MEDIA_ReadEvent* fre = dynamic_cast<MEDIA_ReadEvent *>(event);
        
        
        if (_enabled) {
            // Retime back to source time space
            //
            fre->getTimeInfo()->setVideoFrame(
                fre->getTimeInfo()->videoFrame() - _frameIn);
            
        }
        
        
        
        QObject* srcOwner = fre->getFrameBuffer()
                                ->getBufferMetadata()
                                ->value("node.retime.buffer_src_owner").
                                value<QObject*>();
                                
        QApplication::postEvent(
            srcOwner,
            new MEDIA_ReadEvent(
                fre->getTimeInfo(),
                fre->getNodeIndex(),
                fre->getFrameBuffer(),
                MEDIA_FrameReadEventType));
        
        return true;
    }
}


// ----------


MEDIA_RetimeVideoReadTask::MEDIA_RetimeVideoReadTask(
    MEDIA_RetimeNode* node,
    DS_TimePtr timeInfo,
    DS_BufferPtr destFrame):
    _node(node),
    _timeInfo(timeInfo),
    _destFrame(destFrame)
{
}


void
MEDIA_RetimeVideoReadTask::run()
{
    _node->readVideo(_timeInfo, _destFrame);
}

    
                                                