//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include "RlpExtrevlens/RLQTMEDIA/VideoReader.h"

#include "RlpCore/SEQ/Util.h"

#include "RlpRevlens/MEDIA/Plugin.h"
#include "RlpRevlens/MEDIA/ReadEvent.h"

#include "RlpExtrevlens/RLQTMEDIA/Plugin.h"

RLP_SETUP_LOGGER(ext, RLQTMEDIA, VideoReader)

RLQTMEDIA_VideoReader::RLQTMEDIA_VideoReader(QVariantMap* properties) :
    DS_Node("RLQTMEDIA_VideoReader", properties)
{
    QString ipath = properties->value("video.sequence").toString();
    _seq = SEQ_Util::getSequenceFromTemplateString(ipath);
}

void
RLQTMEDIA_VideoReader::readVideo(DS_TimePtr timeInfo, DS_BufferPtr destFrame, bool postUpdate)
{

    long frameNum = timeInfo->mediaVideoFrame();

    if ((destFrame == nullptr) ||
        (!destFrame->isValid()) || 
        (_properties == nullptr) ||
        (_properties == NULL))
    {

        RLP_LOG_WARN("invalid dest buffer, ABORT READ")
        return;
    }


    if (frameNum >= _seq->size())
    {
        RLP_LOG_ERROR("INVALID FRAME: " << frameNum)
        return;
    }


    QString filePath = _seq->at(frameNum)->fullPath();

    // RLP_LOG_DEBUG(filePath)

    // TODO FIXME: Read directly into a DS_FrameBufferPtr subclass
    //
    QImage img(filePath);

    memcpy(destFrame->data(), img.bits(), destFrame->getDataBytes());


    DS_FrameBuffer* fbuf = dynamic_cast<DS_FrameBuffer*>(destFrame.get());

    fbuf->setColourSpace(DS_FrameBuffer::COLSPACE_NONLINEAR);
    fbuf->setChannelOrder(DS_FrameBuffer::CHANNEL_ORDER_BGRA);
    fbuf->setGLTextureFormat(GL_RGBA);
    fbuf->setGLTextureInternalFormat(GL_RGBA);

    // img.save("/tmp/reader_test.png");

    if (postUpdate)
    {
        QApplication::postEvent(destFrame->getOwner(),
                                new MEDIA_ReadEvent(timeInfo,
                                                    getProperty<int>("index"),
                                                    destFrame,
                                                    MEDIA_FrameReadEventType));
    }
}


void
RLQTMEDIA_VideoReader::scheduleReadTask(
    DS_TimePtr timeInfo,
    DS_BufferPtr destFrame,
    DS_Node::NodeDataType dataType,
    bool optional)
{
    QRunnable* task = new RLQTMEDIA_ReadTask(this, timeInfo, destFrame);

    if (optional)
    {
        _properties->value("video.plugin").
                value<RLQTMEDIA_Plugin *>()->
                getReadThreadPool(dataType)->
                tryStart(task);

    } else
    {
        _properties->value("video.plugin").
                value<RLQTMEDIA_Plugin *>()->
                getReadThreadPool(dataType)->
                start(task);
    }
    
    
}
