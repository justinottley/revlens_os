//
//  Copyright (c) 2014 Justin Ottley. All rights reserved.
//


#include "RlpExtrevlens/RLOIIO/VideoReader.h"
#include "RlpExtrevlens/RLOIIO/Plugin.h"

#include "RlpRevlens/MEDIA/Plugin.h"
#include "RlpRevlens/MEDIA/ReadEvent.h"

#include "RlpCore/SEQ/Util.h"

#include <OpenImageIO/imageio.h>


RLP_SETUP_LOGGER(ext, RLOIIO, VideoReader)

RLOIIO_VideoReader::RLOIIO_VideoReader(QVariantMap* properties) :
    DS_Node("RLOIIO_VideoReader", properties),
    _sseq(nullptr)
{
    if (properties->contains("media.sequence"))
    {
        _sseq = properties->value("media.sequence").value<SEQ_Sequence*>();
        RLP_LOG_DEBUG("Using user-provided sequence:" << _sseq->size())

    } else
    {
        QString ipath = properties->value("video.sequence").toString();
        _seq = SEQ_Util::getSequenceFromTemplateString(ipath);
    }
}


void
RLOIIO_VideoReader::readVideo(DS_TimePtr timeInfo, DS_BufferPtr destFrame, bool postUpdate)
{
    if ((destFrame == nullptr) ||
        (!destFrame->isValid()) || 
        (_properties == nullptr) ||
        (_properties == NULL)) {
    
        RLP_LOG_ERROR("ABORT READ");
        return;
    }
    
    // TODO FIXME: ARRGH FIX THIS!! switch SEQ_Sequence to be non-refcounted?
    SEQ_Sequence* seq = nullptr;
    if (_sseq != nullptr)
    {
        seq = _sseq;
    } else
    {
        seq = _seq.get();
    }


    long frameNum = timeInfo->mediaVideoFrame();
    if (frameNum >= seq->size())
    {

        RLP_LOG_ERROR("INVALID FRAME: " << frameNum << "seq size:" << seq->size())
        return;
    }
    
    // std::cout << "RLOIIO_VideoReader::readVideo() : " << frameNum << std::endl;


    if (_properties->contains("media.frame_offset"))
    {
        int frameOffset = _properties->value("media.frame_offset").toInt();
        frameNum += frameOffset;
        // RLP_LOG_DEBUG("Offsetting" << frameOffset << "->" << frameNum)
    }

    QString filePath = seq->at(frameNum)->fullPath();


    // RLP_LOG_DEBUG("Attempting read:" << filePath)


    auto in = OIIO::ImageInput::open(filePath.toUtf8().constData());
    if (!in)
    {
        RLP_LOG_DEBUG("Error opening file")
        return;
    }

    const OIIO::ImageSpec &spec = in->spec();
    int xres = spec.width;
    int yres = spec.height;
    int nchannels = spec.nchannels;
    // auto pixels = std::unique_ptr<unsigned char[]>(new unsigned char[xres * yres * nchannels]);
    // in->read_image(0, 0, 0, nchannels, OIIO::TypeDesc::UINT8, &pixels[0]);

    DS_FrameBuffer* fbuf = dynamic_cast<DS_FrameBuffer*>(destFrame.get());

    fbuf->getBufferMetadata()->insert("media.full_path", filePath);

    // RLP_LOG_DEBUG(xres << yres << nchannels << fbuf->getChannelCount());

    in->read_image(
        0,
        0,
        0,
        spec.nchannels, // dynamic_cast<DS_FrameBuffer*>(destFrame.get())->channelCount(),
        OIIO::TypeDesc::UINT8,
        destFrame->data()
        // &pixels[0]
    );
    
    in->close();


    DS_FrameBuffer::ColourSpace cspace = DS_FrameBuffer::COLSPACE_NONLINEAR; // DS_FrameBuffer::COLSPACE_LINEAR;
    if (_properties->contains("media.colourspace"))
    {
        QString val = _properties->value("media.colourspace").toString();
        if (val == "XYZ")
        {
            cspace = DS_FrameBuffer::COLSPACE_XYZ;
        } else if (val == "raw")
        {
            //cspace = DS_FrameBuffer::COLSPACE_NONLINEAR;
        }
    }


    fbuf->setColourSpace(cspace);

    if (spec.nchannels == 3)
    {
        fbuf->setChannelOrder(DS_FrameBuffer::CHANNEL_ORDER_RGB);
        fbuf->setGLTextureFormat(GL_RGB);
        fbuf->setGLTextureInternalFormat(GL_RGB8);

    } else if (spec.nchannels == 4)
    {
        fbuf->setChannelOrder(DS_FrameBuffer::CHANNEL_ORDER_RGBA);
        fbuf->setGLTextureFormat(GL_RGBA);
        fbuf->setGLTextureInternalFormat(GL_RGBA8);
    }
    // in->close();


    if (postUpdate)
    {
        //std::cout << "POST UPDATE!!!" << std::endl;
        
        QApplication::postEvent(destFrame->getOwner(),
                                new MEDIA_ReadEvent(timeInfo,
                                                    destFrame,
                                                    MEDIA_FrameReadEventType));
        
        //std::cout << "POST UPDATE DONE!" << std::endl;
    }
    
}


void
RLOIIO_VideoReader::scheduleReadTask(
    DS_TimePtr timeInfo, 
    DS_BufferPtr destBuffer,
    DS_Node::NodeDataType dataType,
    bool optional)
{
    
    QRunnable* task = new RLOIIO_ReadTask(this, timeInfo, destBuffer);
    
    _properties->value("video.plugin").
                value<RLOIIO_Plugin *>()->
                getReadThreadPool(dataType)->
                start(task);
    
}
