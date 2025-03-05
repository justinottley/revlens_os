//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//


#include "RlpExtrevlens/RLQTMEDIA/PreviewMakerNode.h"


#include "RlpRevlens/MEDIA/Plugin.h"
#include "RlpRevlens/DS/FrameBuffer.h"

#include "RlpExtrevlens/RLQTMEDIA/Plugin.h"

RLP_SETUP_LOGGER(ext, RLQTMEDIA, PreviewMakerNode)

RLQTMEDIA_PreviewMakerNode::RLQTMEDIA_PreviewMakerNode() :
    DS_Node("RLQTMEDIA_PreviewMakerNode")
{
}

RLQTMEDIA_PreviewMakerNode::RLQTMEDIA_PreviewMakerNode(QVariantMap* properties) :
    DS_Node("RLQTMEDIA_PreviewMakerNode", properties)
{
    // setProperty("index", 0);
}



RLQTMEDIA_PreviewMakerNode*
RLQTMEDIA_PreviewMakerNode::new_RLQTMEDIA_PreviewMakerNode(DS_NodePtr input)
{
    RLQTMEDIA_PreviewMakerNode* node = new RLQTMEDIA_PreviewMakerNode(input->getPropertiesPtr());
    node->addInput(input);

    return node;
}


void
RLQTMEDIA_PreviewMakerNode::readVideo(DS_TimePtr timeInfo, DS_FrameBufferPtr destFrame, bool postUpdate)
{
    //if (_imageMap.find
        
    RLP_LOG_DEBUG(timeInfo->videoFrame())
    
    
    _inputs[0]->readVideo(timeInfo, destFrame, false);
    
    /*
    int stride = destFrame->getWidth() * destFrame->getPixelSize();
    
    // XXX: mem leak?
    QImage* i = new QImage((const uchar*)destFrame->data(), destFrame->getWidth(), destFrame->getHeight(), stride, QImage::Format_RGB32);
    // QImage w_image(i->scaledToWidth(60));
    
    QImage* scaled_image = new QImage(i->scaledToHeight(60, Qt::SmoothTransformation));
    
    */
    
    // _imageMap[frameNum] = scaled_image;
    
}



/*
QImage*
RLQTMEDIA_PreviewMakerNode::getImage(long frameNum)
{
    // TODO FIXME: error checking
    
    return _imageMap[frameNum];
}
*/

QImage
RLQTMEDIA_PreviewMakerNode::makePreview(long frameNum)
{
    RLP_LOG_DEBUG(frameNum << _inputs.size())

    MEDIA_Plugin* plugin = _inputs[0]->getProperty<MEDIA_Plugin*>("video.plugin");
    QVariantMap frameInfo = _inputs[0]->getProperty<QVariantMap>("video.frame_info.one");
    long sessionFrame = _inputs[0]->getProperty<long>("session.frame");

    DS_FrameBufferPtr n = plugin->getOrCreateFrameBuffer(
        this,
        frameInfo,
        /* force create new framebuffer */ true
    );
    n->getBufferMetadata()->insert("video.preview", true);
    n->reallocate();


    DS_TimePtr ftime = _inputs[0]->makeTime(frameNum + sessionFrame);
    _inputs[0]->readVideo(ftime, n, false);
    
    // int stride = n->getWidth() * n->getPixelSize();
    
    // XXX: mem leak?
    // QImage* i = new QImage((const uchar*)n->data(), n->getWidth(), n->getHeight(), stride, QImage::Format_RGB32);
    // QImage w_image(i->scaledToWidth(60));
    
    // QImage* scaled_image = new QImage(i->scaledToHeight(60, Qt::SmoothTransformation));
    // QImage* scaled_image = new QImage(i->scaledT(60, Qt::SmoothTransformation));
    // QImage scaled_image = i->scaled(150, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    
    QImage image = n->toQImage();

    if (image.width() > 0)
    {
        emit imageReady(frameNum, image);

    } else 
    {
        RLP_LOG_ERROR("Could not convert to image from video framebuffer")
    }
    
    
    return image;
    
}


/*
void
RLQTMEDIA_PreviewMakerNode::scheduleReadTask(long frameNum)
{
    // QRunnable* task = new RLQTMEDIA_PreviewMakerReadTask(this, frameNum);
    // MEDIA_Plugin* plugin = _inputs[0]->getProperty<MEDIA_Plugin*>("video.plugin");
    // plugin->getReadThreadPool(DS_Node::DATA_VIDEO)->start(task);
}
*/


////


RLQTMEDIA_PreviewMakerReadTask::RLQTMEDIA_PreviewMakerReadTask(
    RLQTMEDIA_PreviewMakerNode* reader,
    DS_TimePtr timeInfo):
        _reader(reader),
        _timeInfo(timeInfo)
{
}


void
RLQTMEDIA_PreviewMakerReadTask::run()
{
    _reader->makePreview(_timeInfo->videoFrame());
}
