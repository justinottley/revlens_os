//
//  Copyright (c) 2014 Justin Ottley. All rights reserved.
//

#include "RlpExtrevlens/RLQPDF_MEDIA/VideoReader.h"
#include "RlpExtrevlens/RLQPDF_MEDIA/Plugin.h"

#include "RlpRevlens/DS/Node.h"
#include "RlpRevlens/DS/QImageFrameBuffer.h"

#include "RlpRevlens/MEDIA/Plugin.h"
#include "RlpRevlens/MEDIA/ReadEvent.h"


RLP_SETUP_LOGGER(extrevlens, RLQPDF_MEDIA, VideoReader)

RLQPDF_MEDIA_VideoReader::RLQPDF_MEDIA_VideoReader(QVariantMap* properties) :
    DS_Node("RLQPDF_MEDIA_VideoReader", properties)
{
    QString filePath = properties->value("media.video").toString();

    RLP_LOG_DEBUG("Loading" << filePath)

    // TODO FIXME: switch to thread-local storage for multithreaded read
    _doc = new QPdfDocument();
    _doc->load(filePath);
}


void
RLQPDF_MEDIA_VideoReader::readVideo(DS_TimePtr timeInfo, DS_BufferPtr destFrame, bool postUpdate)
{
    if ((destFrame == nullptr) || (!destFrame->isValid()))
    {
        return;
    }

    long frameNum = timeInfo->mediaVideoFrame();
    // RLP_LOG_DEBUG(frameNum)

    QVariantMap frameInfo = getProperty<QVariantMap>("video.frame_info.one");
    QSize pageSize = QSize(frameInfo.value("width").toInt(), frameInfo.value("height").toInt());

    // RLP_LOG_DEBUG("Rendering at" << pageSize)

    QImage result = _doc->render(frameNum, pageSize).rgbSwapped();

    // RLP_LOG_DEBUG(result.width() << result.height())

    DS_QImageFrameBuffer* df = dynamic_cast<DS_QImageFrameBuffer*>(destFrame.get());

    df->getQImage()->swap(result);


    if (postUpdate)
    {
        QApplication::postEvent(destFrame->getOwner(),
                                new MEDIA_ReadEvent(timeInfo,
                                                    destFrame,
                                                    MEDIA_FrameReadEventType));
    }

}


QImage
RLQPDF_MEDIA_VideoReader::renderPage(int pageNum)
{
    QSizeF pageSize = _doc->pagePointSize(pageNum);
    QImage result = _doc->render(pageNum, QSize(pageSize.width(), pageSize.height()));
    return result;
}


void
RLQPDF_MEDIA_VideoReader::scheduleReadTask(
    DS_TimePtr timeInfo,
    DS_FrameBufferPtr destFrame,
    DS_Node::NodeDataType dataType,
    bool optional)
{

    QRunnable* task = new RLQPDF_MEDIA_ReadTask(this, timeInfo, destFrame);

    _properties->value("video.plugin").
                 value<RLQPDF_MEDIA_Plugin *>()->
                 getReadThreadPool(dataType)->
                 start(task);

}
