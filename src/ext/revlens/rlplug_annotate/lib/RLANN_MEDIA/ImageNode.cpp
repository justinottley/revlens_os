//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include "RlpExtrevlens/RLANN_MEDIA/ImageNode.h"

#include "RlpExtrevlens/RLANN_DS/FrameBuffer.h"

#include "RlpRevlens/MEDIA/Plugin.h"
#include "RlpRevlens/MEDIA/ReadEvent.h"


RLP_SETUP_EXT_LOGGER(RLANN_MEDIA, ImageNode)

RLANN_MEDIA_ImageNode::RLANN_MEDIA_ImageNode(QVariantMap* properties):
    DS_Node("RLANN_MEDIA_ImageNode", properties)
{
    QVariantMap frameInfo = properties->value("video.frame_info.one").toMap();
    _img = QImage(
        frameInfo.value("width").toInt(),
        frameInfo.value("height").toInt(),
        QImage::Format_ARGB32
    );
    _img.fill(Qt::transparent);
}



void
RLANN_MEDIA_ImageNode::readVideo(DS_TimePtr timeInfo, DS_BufferPtr destFrame, bool postUpdate)
{
    // RLP_LOG_DEBUG(timeInfo->videoFrame());

    memcpy(destFrame->data(), _img.bits(), destFrame->getDataBytes());

    if (postUpdate) {
        QApplication::postEvent(
            destFrame->getOwner(),
            new MEDIA_ReadEvent(timeInfo,
                                getProperty<int>("index"),
                                destFrame,
                                MEDIA_FrameReadEventType)
        );
    }
}


void
RLANN_MEDIA_ImageNode::scheduleReadTask(DS_TimePtr timeInfo,
                                  DS_BufferPtr destFrame,
                                  DS_Node::NodeDataType dataType,
                                  bool optional)
{
    
    QRunnable* task = new RLANN_MEDIA_ImageReadTask(this, timeInfo, destFrame);

    QThreadPool* tpool = _properties->value("video.plugin").
                                      value<MEDIA_Plugin *>()->
                                      getReadThreadPool(dataType);
    
    if ((optional) && (timeInfo->mediaVideoFrame() != 0)) {
        tpool->tryStart(task);
        
    } else {
        tpool->start(task);
    }

}


// ------------------------------------------------------------

RLANN_MEDIA_ImageReadTask::RLANN_MEDIA_ImageReadTask(RLANN_MEDIA_ImageNode* node,
                                                     DS_TimePtr timeInfo,
                                                     DS_BufferPtr destFrame) :
    _node(node),
    _timeInfo(timeInfo),
    _destFrame(destFrame)
{
}


void
RLANN_MEDIA_ImageReadTask::run()
{
    _node->readVideo(_timeInfo, _destFrame);
}