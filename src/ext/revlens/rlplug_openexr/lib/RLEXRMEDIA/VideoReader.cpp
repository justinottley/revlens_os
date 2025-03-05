//
//  Copyright (c) 2014 Justin Ottley. All rights reserved.
//


#include "RlpExtrevlens/RLEXRMEDIA/VideoReader.h"
#include "RlpExtrevlens/RLEXRDS/FrameBuffer.h"
#include "RlpExtrevlens/RLEXRMEDIA/Plugin.h"

#include "RlpExtrevlens/RLEXRMEDIA/ExrIncludes.h"

#include "RlpRevlens/MEDIA/Plugin.h"
#include "RlpRevlens/MEDIA/ReadEvent.h"

#include "RlpCore/SEQ/Util.h"

RLP_SETUP_LOGGER(ext, RLEXRMEDIA, VideoReader)

RLEXRMEDIA_VideoReader::RLEXRMEDIA_VideoReader(QVariantMap* properties) :
    DS_Node("RLEXRMEDIA_VideoReader", properties)
{
    QString ipath = properties->value("video.sequence").toString();
    _seq = SEQ_Util::getSequenceFromTemplateString(ipath);
}


void
RLEXRMEDIA_VideoReader::readVideo(DS_TimePtr timeInfo, DS_BufferPtr destFrame, bool postUpdate)
{

    RLEXRDS_FrameBuffer* exrFrameBuffer = dynamic_cast<RLEXRDS_FrameBuffer* >(destFrame.get());
    
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
    
    // std::cout << "RLEXRMEDIA_VideoReader::readVideo() : " << frameNum << std::endl;



    QString filePath = _seq->at(frameNum)->fullPath();

    QByteArray mpathba = filePath.toLocal8Bit();
    const char* srcPath = mpathba.data();
    
    RLP_LOG_DEBUG("Attempting read:" << filePath)

    exrFrameBuffer->getBufferMetadata()->insert("exr.path", filePath);

    // EXR STUFF
    
    
    //Imf::RgbaInputFile file(fileName.toStdString().c_str());
    Imf::InputFile file(srcPath);
    Imath::Box2i dw = file.header().dataWindow();
    
    // std::cout << "min y: " << dw.min.y << std::endl;
    // std::cout << "min x: " << dw.min.x << std::endl;
    
    //int width  = dw.max.x - dw.min.x + 1;
    //int height = dw.max.y - dw.min.y + 1;
    
    // RLEXRDS_FrameBuffer* fbuf = dynamic_cast<RLEXRDS_FrameBuffer*>(destFrame.get());
    
    // Imf::Array2D<Imf::Rgba> pixels;
    
    // std::cout << width << " " << height << std::endl;
    // std::cout << destFrame->getDataBytes() << std::endl;
    
    // Imf::Rgba* pixels = reinterpret_cast<Imf::Rgba *>(destFrame->getDataBytes());
    // Imf::Array2D<Imf::Rgba>* pixels = reinterpret_cast<Imf::Array2D<Imf::Rgba> *>(destFrame->data());
    
    // pixels->resizeErase(height, width);
    // Imf::Array2D<Imf::Rgba>* pixels = destFrame->getPixels();
    
    // pixels->resizeErase(height, width);
        
    file.setFrameBuffer(exrFrameBuffer->getFrameBuffer());

    
    file.readPixels(dw.min.y, dw.max.y);
    
    

    int channelCount = exrFrameBuffer->getBufferMetadata()->value("exr.channelCount").value<int>();
    DS_FrameBuffer::ChannelOrder chanOrder;
    
    
    
    if (channelCount == 3) {
        chanOrder = DS_FrameBuffer::CHANNEL_ORDER_RGB;
        
    } else if (channelCount == 4) {
        chanOrder = DS_FrameBuffer::CHANNEL_ORDER_RGBA;
        
    } else {
        RLP_LOG_ERROR("UNSUPPORTED CHANNEL COUNT: " << channelCount)
    }
    
    
    
    exrFrameBuffer->setColourSpace(DS_FrameBuffer::COLSPACE_LINEAR);

    // TODO FIXME: Force channel order RGB to get around some alpha issues
    // Expose as configrable option
    //
    // exrFrameBuffer->setChannelOrder(chanOrder);
    // exrFrameBuffer->setChannelOrder(DS_FrameBuffer::CHANNEL_ORDER_RGB);
    // exrFrameBuffer->setChannelOrder(DS_FrameBuffer::CHANNEL_ORDER_BGRA);

    exrFrameBuffer->setChannelOrder(DS_FrameBuffer::CHANNEL_ORDER_RGB); // 


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
RLEXRMEDIA_VideoReader::scheduleReadTask(
    DS_TimePtr timeInfo, 
    DS_BufferPtr destBuffer,
    DS_Node::NodeDataType dataType,
    bool optional)
{
    
    QRunnable* task = new RLEXRMEDIA_ReadTask(this, timeInfo, destBuffer);
    
    _properties->value("video.plugin").
                value<RLEXRMEDIA_Plugin *>()->
                getReadThreadPool(dataType)->
                start(task);
    
}
