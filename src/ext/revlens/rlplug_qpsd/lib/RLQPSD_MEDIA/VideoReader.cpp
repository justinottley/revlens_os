//
//  Copyright (c) 2014 Justin Ottley. All rights reserved.
//

#include "RlpExtrevlens/RLQPSD_MEDIA/VideoReader.h"
#include "RlpExtrevlens/RLQPSD_MEDIA/Plugin.h"

#include "RlpExtrevlens/QPSD/qpsdhandler.h"

#include "RlpRevlens/DS/Node.h"
#include "RlpRevlens/MEDIA/Plugin.h"
#include "RlpRevlens/MEDIA/ReadEvent.h"


RLP_SETUP_LOGGER(extrevlens, RLQPSD_MEDIA, VideoReader)

RLQPSD_MEDIA_VideoReader::RLQPSD_MEDIA_VideoReader(QVariantMap* properties) :
    DS_Node("RLQPSD_MEDIA_VideoReader", properties)
{
}


void
RLQPSD_MEDIA_VideoReader::readVideo(DS_TimePtr timeInfo, DS_BufferPtr destFrame, bool postUpdate)
{
    if ((destFrame == nullptr) || (!destFrame->isValid()))
    {
        return;
    }

    long frameNum = timeInfo->mediaVideoFrame();
    RLP_LOG_DEBUG(frameNum)

    QString filePath = getProperty<QString>("media.video"); // .replace("{f}", fstr);

    // READ HERE
    QPsdHandler psdHandler;
    QFile f(filePath);

    f.open(QIODevice::ReadOnly);

    if (!psdHandler.canRead(&f))
    {
        RLP_LOG_ERROR("PSD Read failed for" << filePath)
        return;
    }


    QImage image;

    psdHandler.setDevice(&f);
    psdHandler.read(&image);

    DS_FrameBuffer* df = dynamic_cast<DS_FrameBuffer*>(destFrame.get());

    if ((image.width() != df->getWidth()) ||
        (image.height() != df->getHeight()))
    {

        RLP_LOG_WARN("frame size mismatch, resizing frame buffer")

        int width = image.width();
        int height = image.height();
        int channelCount = image.bitPlaneCount() / 8;
        int pixelSize = image.depth() / 8;
        int byteCount = width * height * pixelSize;

        QVariantMap frameInfo;

        frameInfo.insert("width", width);
        frameInfo.insert("height", height);
        frameInfo.insert("pixelSize", pixelSize);
        frameInfo.insert("channelCount", channelCount); // TODO FIXME: what should this be? is this used?
        frameInfo.insert("byteCount", byteCount);

        destFrame->reallocate(frameInfo);

    }

    memcpy(destFrame->data(), image.bits(), destFrame->getDataBytes());

    if (postUpdate)
    {

        QApplication::postEvent(destFrame->getOwner(),
                                new MEDIA_ReadEvent(timeInfo,
                                                    0, //getProperty<int>("index", 0),
                                                    destFrame,
                                                    MEDIA_FrameReadEventType));
    }

}


void
RLQPSD_MEDIA_VideoReader::scheduleReadTask(
    DS_TimePtr timeInfo,
    DS_FrameBufferPtr destFrame,
    DS_Node::NodeDataType dataType,
    bool optional)
{

    QRunnable* task = new RLQPSD_MEDIA_ReadTask(this, timeInfo, destFrame);

    _properties->value("video.plugin").
                 value<RLQPSD_MEDIA_Plugin *>()->
                 getReadThreadPool(dataType)->
                 start(task);

}
