//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include "RlpExtrevlens/RLMEDIA/VideoReader.h"
#include "RlpExtrevlens/RLMEDIA/Plugin.h"

#include "RlpRevlens/MEDIA/Plugin.h"
#include "RlpRevlens/MEDIA/ReadEvent.h"


RLP_SETUP_LOGGER(ext, RLMEDIA, VideoReader)

RLMEDIA_VideoReader::RLMEDIA_VideoReader(QVariantMap* properties) :
    DS_Node("RLMEDIA_VideoReader", properties),
    _videoCodec("")
{
    _videoCodec = properties->value("video.codec").toString();
}


void
RLMEDIA_VideoReader::_readVideo(qlonglong frameNum, DS_QImageFrameBuffer* fbuf)
{
    // RLP_LOG_DEBUG(frameNum)

    if (!_mediaStorage.hasLocalData())
    {
        QString rlmPath = _properties->value("media.video").toString();
        RLP_LOG_DEBUG("Creating thread-local connection for" << rlmPath)
        EdbMedia_Controller* cntrl = new EdbMedia_Controller(rlmPath);
        _mediaStorage.setLocalData(cntrl);
    }

    EdbMedia_Controller* cntrl = _mediaStorage.localData();

    QByteArray fdata = cntrl->getFrameData("main.full", frameNum + 1);

    if (_videoCodec == "jpg")
    {
        fbuf->getQImage()->loadFromData(fdata, "jpg");

    } else if (_videoCodec == "raw")
    {
        if (fdata.size() == fbuf->getDataBytes())
        {
            memcpy(fbuf->data(), fdata.data(), fbuf->getDataBytes());
        } else
        {
            RLP_LOG_ERROR("bad raw frame:" << frameNum)
        }
    }
}


void
RLMEDIA_VideoReader::readVideo(DS_TimePtr timeInfo, DS_BufferPtr destFrame, bool postUpdate)
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

    if (_videoCodec == "")
    {
        RLP_LOG_WARN("codec not set, aborting")
        return;
    }

    DS_QImageFrameBuffer* fbuf = dynamic_cast<DS_QImageFrameBuffer*>(destFrame.get());

    if ((fbuf->getBufferMetadata()->contains("video.display_immediate")) &&
        (fbuf->getBufferMetadata()->value("video.display_immediate") != (int)DS_FrameBuffer::FRAME_QUALITY_FULL))
    {
        // no direct scrubbing on ios, unstable - rely on thumbnail cache
        #ifndef SCAFFOLD_IOS
        _readVideo(frameNum, fbuf);
        #endif

    } else
    {
        _readVideo(frameNum, fbuf);
    }


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
RLMEDIA_VideoReader::scheduleReadTask(
    DS_TimePtr timeInfo,
    DS_BufferPtr destFrame,
    DS_Node::NodeDataType dataType,
    bool optional)
{
    QRunnable* task = new RLMEDIA_ReadTask(this, timeInfo, destFrame);

    // RLP_LOG_DEBUG(optional)

    if (optional)
    {
        // RLP_LOG_DEBUG("optional start")

        _properties->value("video.plugin").
                value<RLMEDIA_Plugin *>()->
                getReadThreadPool(dataType)->
                tryStart(task);

    } else
    {
        _properties->value("video.plugin").
                value<RLMEDIA_Plugin *>()->
                getReadThreadPool(dataType)->
                start(task);
    }
    
    
}
