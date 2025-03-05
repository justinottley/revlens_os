//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include "RlpExtrevlens/RLMEDIA/VideoReader.h"
#include "RlpExtrevlens/RLMEDIA/Plugin.h"

#include "RlpRevlens/MEDIA/Plugin.h"
#include "RlpRevlens/MEDIA/ReadEvent.h"
#include "RlpRevlens/DS/QImageFrameBuffer.h"


// #include <turbojpeg.h>


RLP_SETUP_LOGGER(ext, RLMEDIA, VideoReader)

RLMEDIA_VideoReader::RLMEDIA_VideoReader(QVariantMap* properties) :
    DS_Node("RLMEDIA_VideoReader", properties),
    _videoCodec("")
{
    _videoCodec = properties->value("video.codec").toString();
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


    // RLP_LOG_DEBUG(frameNum)

    // #ifdef SCAFFOLD_IOS
    // QString appRoot = getenv("APP_ROOT");
    // QString rlmPath = QString("%1/Videos/moana_2.rlm").arg(appRoot);
    // #else
    // QString rlmPath = "/Users/justinottley/dev/revlens_root/sp_revsix/moana_2.rlm";
    // #endif 

    if (!_mediaStorage.hasLocalData())
    {
        QString rlmPath = _properties->value("media.video").toString();
        RLP_LOG_DEBUG("Creating thread-local connection for" << rlmPath)
        EdbMedia_Controller* cntrl = new EdbMedia_Controller(rlmPath);
        _mediaStorage.setLocalData(cntrl);
    }


    EdbMedia_Controller* cntrl = _mediaStorage.localData();

    QByteArray fdata = cntrl->getFrameData("main.full", frameNum + 1);



    /*
    tjhandle tjInstance = NULL;
    if ((tjInstance = tj3Init(TJINIT_DECOMPRESS)) == NULL)
    {
        RLP_LOG_ERROR("error initializing turbojpeg")
        // return 1;
    }

    RLP_LOG_DEBUG("frame:" << frameNum + 1 << "size:" << fdata.size());

    if (tj3DecompressHeader(tjInstance, (unsigned char*)fdata.constData(), fdata.size()) < 0)
    {
        RLP_LOG_ERROR("could not read jpeg header");
    } else
    {
        RLP_LOG_DEBUG("HEADER READ OK")
        int width = tj3Get(tjInstance, TJPARAM_JPEGWIDTH);
        int height = tj3Get(tjInstance, TJPARAM_JPEGHEIGHT);
        int precision = tj3Get(tjInstance, TJPARAM_PRECISION);
        int sampleSize = (precision <= 8 ? sizeof(unsigned char) : sizeof(short));
        int colorspace = tj3Get(tjInstance, TJPARAM_COLORSPACE);

        int pixelFormat = TJPF_UNKNOWN;
        if (colorspace == TJCS_CMYK || colorspace == TJCS_YCCK)
        {
             pixelFormat = TJPF_CMYK;
             RLP_LOG_DEBUG("PIXELFORMAT CMYK")
        } else {
            pixelFormat = TJPF_RGB;
            RLP_LOG_DEBUG("PIXELFORMAT RGB")
        }
            
        RLP_LOG_DEBUG(width << height << precision << sampleSize)

        int dstSize = sizeof(unsigned char) * width * height *
                                tjPixelSize[pixelFormat] * sampleSize;

        RLP_LOG_DEBUG("dstSize:" << dstSize << "destFrame:" << destFrame->getDataBytes())

    //     unsigned char* dstBuf;
    //     dstBuf =
    //    (unsigned char *)malloc(sizeof(unsigned char) * width * height *
    //                            tjPixelSize[pixelFormat] * sampleSize);

        if (tj3Decompress8(tjInstance, (unsigned char*)fdata.constData(), fdata.size(), (unsigned char*)destFrame->data(), 0,
                        pixelFormat) < 0)
        {
            RLP_LOG_ERROR("Error decompressing");
        } else
        {
            RLP_LOG_DEBUG("DECOMPRESS OK")
            // QByteArray df((char *)destFrame->data(), destFrame->getDataBytes());
            // int fnum = frameNum + 1;
            // QString frameOutPath = QString("/tmp/test_img/%1.jpg").arg(fnum);

            // RLP_LOG_DEBUG(frameOutPath);
            // QImage testdec = QImage((uchar*)destFrame->data(), width, height, QImage::Format_RGB32);
            // RLP_LOG_DEBUG(testdec.width() << testdec.height());
            // bool sresult = testdec.save(frameOutPath, "jpg");
            // RLP_LOG_DEBUG("Save result:" << sresult);
        }
        
    }
    // THROW_TJ("reading JPEG header");
    
    
    tj3Destroy(tjInstance);
    

    // return;
    */

    DS_QImageFrameBuffer* fbuf = dynamic_cast<DS_QImageFrameBuffer*>(destFrame.get());

    if (_videoCodec == "jpg")
    {

        // RLP_LOG_DEBUG(fdata.size());
        fbuf->getQImage()->loadFromData(fdata, "jpg");


        // int fnum = frameNum + 1;
        // QString frameOutPath = QString("/tmp/test_img/%1.jpg").arg(fnum);
        
        // RLP_LOG_DEBUG(fbuf->getWidth() << fbuf->getHeight())

        // QImage fi = QImage::fromData(fdata, "jpg");
        // fbuf->getQImage()->swap(fi);
        // RLP_LOG_DEBUG(fbuf->getWidth() << fbuf->getHeight() << fbuf->getDataBytes());
        // RLP_LOG_DEBUG(fi->width() << fi->height() << fi->format());
        // fbuf->getQImage()->save(frameOutPath, "jpg");
        //fi.save(frameOutPath, "jpg");
        

        // RLP_LOG_DEBUG(frameOutPath);
        //  

        //  if (fi.sizeInBytes() == destFrame->getDataBytes())
        //  {
        //     memcpy(destFrame->data(), fi.bits(), destFrame->getDataBytes());
        //  } else
        //  {
        //     RLP_LOG_ERROR("bad jpg frame:" << frameNum << "read:" << fi.sizeInBytes() << "buffer size:" << destFrame->getDataBytes())
        //  }



    } else if (_videoCodec == "raw")
    {
        if (fdata.size() == destFrame->getDataBytes())
        {
            memcpy(destFrame->data(), fdata.data(), destFrame->getDataBytes());
        } else
        {
            RLP_LOG_ERROR("bad raw frame:" << frameNum)
        }
    }
   
    // RLP_LOG_DEBUG("data size:" << fdata.size() << "dest data bytes:" << destFrame->getDataBytes())

    // 

    // RLP_LOG_DEBUG("Warning: bad frame:" << frameNum << "size:" << fdata.size() << "vs" << destFrame->getDataBytes())


    // DS_FrameBuffer* fbuf = dynamic_cast<DS_FrameBuffer*>(destFrame.get());

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

    } else {
        _properties->value("video.plugin").
                value<RLMEDIA_Plugin *>()->
                getReadThreadPool(dataType)->
                start(task);
    }
    
    
}
