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
    DS_Node("RLOIIO_VideoReader", properties)
{
    QString ipath = properties->value("video.sequence").toString();
    _seq = SEQ_Util::getSequenceFromTemplateString(ipath);
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
    
    long frameNum = timeInfo->mediaVideoFrame();
    if (frameNum >= _seq->size()) {

        RLP_LOG_ERROR("INVALID FRAME: " << frameNum);
        return;
    }
    
    // std::cout << "RLOIIO_VideoReader::readVideo() : " << frameNum << std::endl;



    QString filePath = _seq->at(frameNum)->fullPath();
    // "/home/justinottley/Videos/jpg_example/bbb_trailer_480p.0001.jpg"; // mpathba.data();
    // 

    RLP_LOG_DEBUG("Attempting read:" << filePath)


    auto in = OIIO::ImageInput::open(filePath.toUtf8().constData());
    if (!in) {
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

    RLP_LOG_DEBUG(xres << yres << nchannels << fbuf->getChannelCount());

    
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
    /*
    std::unique_ptr<OIIO::ImageOutput> out = OIIO::ImageOutput::create("/tmp/test_out.jpg");
    if (!out)
        return;  // error
    OIIO::ImageSpec ospec(xres, yres, nchannels, OIIO::TypeDesc::UINT8);
    out->open("/tmp/test_out.jpg", ospec);
    out->write_image(OIIO::TypeDesc::UINT8, destFrame->data()); // destFrame->data()// pixels);
    out->close();
    */

    // QImage i((uchar*)destFrame->data(), xres, yres, QImage::Format_RGB888);
    // i.save("/tmp/test_out_qimage_1.jpg");
    


    
    if (spec.nchannels == 3)
    {
        fbuf->setChannelOrder(DS_FrameBuffer::CHANNEL_ORDER_RGB);
        fbuf->setColourSpace(DS_FrameBuffer::COLSPACE_LINEAR);
        fbuf->setGLTextureFormat(GL_RGB);
        fbuf->setGLTextureInternalFormat(GL_RGB8);
    } else if (spec.nchannels == 4)
    {
        fbuf->setChannelOrder(DS_FrameBuffer::CHANNEL_ORDER_RGBA);
        fbuf->setColourSpace(DS_FrameBuffer::COLSPACE_LINEAR);
        fbuf->setGLTextureFormat(GL_RGBA);
        fbuf->setGLTextureInternalFormat(GL_RGBA8);
    }
    // in->close();


    if (postUpdate) {
        
        //std::cout << "POST UPDATE!!!" << std::endl;
        
        QApplication::postEvent(destFrame->getOwner(),
                                new MEDIA_ReadEvent(timeInfo,
                                                    getProperty<int>("index"),
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
