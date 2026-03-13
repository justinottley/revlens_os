//
//  Copyright (c) 2014 Justin Ottley. All rights reserved.
//

#include "RlpRevlens/MEDIA/Factory.h"
#include "RlpRevlens/MEDIA/CacheNode.h"
#include "RlpRevlens/MEDIA/ReadEvent.h"

#include "RlpCore/UTIL/Convert.h"


RLP_SETUP_LOGGER(revlens, MEDIA, CacheNode)
RLP_SETUP_LOGGER(revlens, MEDIA, ThumbnailCacheTask)

MEDIA_CacheNode::MEDIA_CacheNode(QVariantMap* properties, DS_NodePtr nodeIn):
    DS_Node("MEDIA_Cache", properties),
    _thumbCache(nullptr),
    _thumbCacheState(THUMB_CACHE_DISABLED)
{

    _mediaCacheDir = DS_Session::cacheDir(graph()->uuid().toString());
    _mediaCachePixelDataPath = QString("%1/pixeldata").arg(_mediaCacheDir);
    _mediaCachePosDataPath = QString("%1/posdata").arg(_mediaCacheDir);


    #ifdef SCAFFOLD_IOS
    RLP_LOG_DEBUG("Caching to:" << _mediaCacheDir);
    _thumbCacheState = THUMB_CACHE_UNINITIALIZED;
    #endif

    _inputs.clear();
    _inputs.push_back(nodeIn);

    initLocalProperties();
}


void
MEDIA_CacheNode::initLocalProperties()
{
    // RLP_LOG_DEBUG("")
    #ifndef SCAFFOLD_IOS
    return;
    #endif

    DS_Graph* graph = _properties->value("graph").value<DS_Graph*>();
    connect(
        graph,
        &DS_Graph::dataReady,
        this,
        &MEDIA_CacheNode::onGraphDataReady,
        Qt::UniqueConnection
    );
}


void
MEDIA_CacheNode::onGraphDataReady(QString dataType, QVariantMap data)
{
    // RLP_LOG_DEBUG(dataType)

    #ifndef SCAFFOLD_IOS
    return;
    #endif

    if (dataType == "video_source")
    {
        ThumbCacheState tcs = thumbCacheState();
        if (tcs == MEDIA_CacheNode::THUMB_CACHE_UNINITIALIZED)
        {
            scheduleThumbnailGen(data);
        }
    }
}


void
MEDIA_CacheNode::setThumbCacheState(ThumbCacheState tcstate)
{
    // RLP_LOG_DEBUG("")

    QMutexLocker l(&_tlock);
    _thumbCacheState = tcstate;
}


MEDIA_CacheNode::ThumbCacheState
MEDIA_CacheNode::thumbCacheState()
{
    // RLP_LOG_DEBUG("")

    QMutexLocker l(&_tlock);

    return _thumbCacheState;
}


void
MEDIA_CacheNode::scheduleThumbnailGen(QVariantMap data)
{
    // RLP_LOG_DEBUG(data)


    if (_thumbCache != nullptr)
    {
        RLP_LOG_ERROR("Skipping, thumbnail cache exists")
        return;
    }

    setThumbCacheState(MEDIA_CacheNode::THUMB_CACHE_RUNNING);

    DS_NodePtr reader = data.value("reader").value<DS_NodePtr>();

    _thumbCache = new ThumbCacheMap();
    _thumbCacheTask = new MEDIA_ThumbnailCacheTask(this, reader);
    _thumbCacheTask->setAutoDelete(false);

    _thpool.setMaxThreadCount(1);

    _thpool.start(_thumbCacheTask);
    _ttimer.callOnTimeout(this, &MEDIA_CacheNode::continueThumbnailGen);
    _ttimer.setInterval(100);
    _ttimer.start();
}


void
MEDIA_CacheNode::continueThumbnailGen()
{
    // RLP_LOG_DEBUG("")

    ThumbCacheState tcs = thumbCacheState();

    if (tcs == MEDIA_CacheNode::THUMB_CACHE_RUNNING)
    {
        _thpool.tryStart(_thumbCacheTask);
    } else if (tcs == MEDIA_CacheNode::THUMB_CACHE_READY)
    {
        _ttimer.stop();

        QString posDataPath = QString("%1/posdata").arg(_mediaCacheDir);
        RLP_LOG_DEBUG("Reading disk cache pos data from" << posDataPath);

        QFile f(posDataPath);
        if (f.open(QIODevice::ReadOnly))
        {
            RLP_LOG_DEBUG("cache pos data OK")
        }

        QString serializedPosData = f.readAll();
        _posInfo = UTIL_Convert::fromJsonString(serializedPosData);
    }

    DS_Graph* graph = getProperty<DS_Graph*>("graph");
    qlonglong numFrames = getProperty<qlonglong>("media.frame_count");

    QVariantMap data;
    data.insert("thumbnail_cache.count", _thumbCacheTask->fnum());
    data.insert("thumbnail_cache.total", numFrames);
    graph->emitDataReady("thumbnail_cache", data);
}


void
MEDIA_CacheNode::readVideo(DS_TimePtr timeInfo, DS_BufferPtr destFrame, bool postUpdate)
{
    #ifndef SCAFFOLD_IOS
    return _inputs[0]->readVideo(timeInfo, destFrame, postUpdate);
    #endif

    // RLP_LOG_DEBUG(timeInfo->videoFrame())

    ThumbCacheState tcs = thumbCacheState();

    if ((destFrame->getBufferMetadata()->contains("video.frame_quality")) &&
         (destFrame->getBufferMetadata()->value("video.frame_quality") != (int)DS_FrameBuffer::FRAME_QUALITY_FULL))
    {

        if (tcs != MEDIA_CacheNode::THUMB_CACHE_READY)
        {
            return;
        }

        qlonglong frameNum = timeInfo->mediaVideoFrame();

        // RLP_LOG_DEBUG(frameNum)
        
        QString strFrame = QString::number(frameNum);
        if (_posInfo.contains(strFrame))
        {
            // RLP_LOG_DEBUG("GOT POS INFO FOR" << strFrame)

            QVariantMap posInfo = _posInfo.value(strFrame).toMap();

            qint64 filePos = posInfo.value("pos").toInt();
            qint64 numBytes = posInfo.value("size").toInt();
            int imgWidth = posInfo.value("w").toInt();
            int imgHeight = posInfo.value("h").toInt();


            QFile f(_mediaCachePixelDataPath);
            f.open(QIODevice::ReadOnly);
            f.seek(filePos);
            
            QByteArray jpgData;
            jpgData.resize(numBytes);
            f.read(jpgData.data(), numBytes);

            QImage i;
            bool readOk = i.loadFromData(jpgData, "JPG");
            i = i.rgbSwapped();

            if (!readOk)
            {
                RLP_LOG_ERROR("Image read failed")
            }

            QVariantMap frameInfo;
            frameInfo.insert("width", imgWidth);
            frameInfo.insert("height", imgHeight);
            frameInfo.insert("pixelSize", 1);
            frameInfo.insert("channelCount", 4);
            frameInfo.insert("byteCount", numBytes);
            // frameInfo.insert("hasAlpha", i.hasAlphaChannel());

            MEDIA_Plugin* plugin = getProperty<MEDIA_Plugin*>("video.plugin");
            DS_FrameBufferPtr p = plugin->getOrCreateFrameBuffer(this, frameInfo);

            p->reallocate();
            p->setColourSpace(DS_FrameBuffer::COLSPACE_NONLINEAR);
            // p->setChannelOrder(DS_FrameBuffer::CHANNEL_ORDER_BGRA);
            p->setGLTextureFormat(GL_RGBA);
            p->setGLTextureInternalFormat(GL_RGBA);

            // memcpy(p->data(), imageData.data(), p->getDataBytes());
            // f.read((char *)p->data(), numBytes);
            memcpy(p->data(), i.bits(), p->getDataBytes());

            destFrame->appendAuxBuffer(p);

            if (postUpdate)
            {

                // RLP_LOG_DEBUG("POST UPDATE "  << timeInfo->mediaVideoFrame() << destFrame->getOwner())

                QApplication::postEvent(destFrame->getOwner(),
                                        new MEDIA_ReadEvent(timeInfo,
                                                            destFrame,
                                                            MEDIA_FrameReadEventType));
            }

            return;

        }



        /*

        ThumbCacheMap* tcm = _thumbCache;
        if (tcm == nullptr)
        {
            RLP_LOG_ERROR("no thumb cache")
        }

        if ((tcm != nullptr) && (tcm->contains(frameNum)))
        {
            QImage i = tcm->value(frameNum);

            // RLP_LOG_DEBUG("Found thumb for frame:" << frameNum << i.width() << "x" << i.height())

            QVariantMap frameInfo;
            frameInfo.insert("width", i.width());
            frameInfo.insert("height", i.height());
            frameInfo.insert("pixelSize", 1);
            frameInfo.insert("channelCount", 4);
            frameInfo.insert("byteCount", i.sizeInBytes());
            frameInfo.insert("hasAlpha", i.hasAlphaChannel());

            MEDIA_Plugin* plugin = getProperty<MEDIA_Plugin*>("video.plugin");
            DS_FrameBufferPtr p = plugin->getOrCreateFrameBuffer(this, frameInfo);

            p->reallocate();
            p->setColourSpace(DS_FrameBuffer::COLSPACE_NONLINEAR);
            // p->setChannelOrder(DS_FrameBuffer::CHANNEL_ORDER_BGRA);
            p->setGLTextureFormat(GL_RGBA);
            p->setGLTextureInternalFormat(GL_RGBA);

            memcpy(p->data(), i.bits(), p->getDataBytes());

            destFrame->appendAuxBuffer(p);

            if (postUpdate)
            {

                // RLP_LOG_DEBUG("POST UPDATE "  << timeInfo->mediaVideoFrame() << destFrame->getOwner())

                QApplication::postEvent(destFrame->getOwner(),
                                        new MEDIA_ReadEvent(timeInfo,
                                                            destFrame,
                                                            MEDIA_FrameReadEventType));
            }

            // RLP_LOG_DEBUG("dest frame aux buffer list size:" << destFrame->numAuxBuffers(false));
            
        } else
        {
            RLP_LOG_WARN("frame not found in thumb cache:" << frameNum)
        }

        */
    
    } else
    {
        _inputs[0]->readVideo(timeInfo, destFrame, postUpdate);
    }
}


void
MEDIA_CacheNode::readAudio(DS_TimePtr timeInfo, DS_BufferPtr destBuffer)
{
    return _inputs[0]->readAudio(timeInfo, destBuffer);
}


void
MEDIA_CacheNode::scheduleReadTask(
    DS_TimePtr timeInfo,
    DS_BufferPtr destFrame,
    NodeDataType dataType,
    bool optional)
{
    QRunnable* task = new MEDIA_CacheReadTask(this, timeInfo, destFrame);
    QThreadPool* tpool = _properties->value("video.plugin").
                                      value<MEDIA_Plugin *>()->
                                      getReadThreadPool(dataType);

    if (optional)
    {
        tpool->tryStart(task);
    } else
    {
        tpool->start(task);
    }
}


// -------------

MEDIA_CacheReadTask::MEDIA_CacheReadTask(
    MEDIA_CacheNode* cacheNode,
    DS_TimePtr timeInfo,
    DS_BufferPtr destFrame
):
    _cacheNode(cacheNode),
    _timeInfo(timeInfo),
    _destFrame(destFrame)
{
}


void
MEDIA_CacheReadTask::run()
{
    _cacheNode->readVideo(_timeInfo, _destFrame);
}


// ---------------


MEDIA_ThumbnailCacheTask::MEDIA_ThumbnailCacheTask(MEDIA_CacheNode* cacheNode, DS_NodePtr reader):
    _cacheNode(cacheNode),
    _reader(reader),
    _fnum(0)
{
    MEDIA_Plugin* plugin = _reader->getProperty<MEDIA_Plugin*>("video.plugin");
    QVariantMap* mprops = _reader->getPropertiesPtr();

    _thumbReader = plugin->createVideoReader(mprops);
    if (_thumbReader == nullptr)
    {
        RLP_LOG_ERROR("Thumb reader creation failed, aborting")
        _cacheNode->setThumbCacheState(MEDIA_CacheNode::THUMB_CACHE_DISABLED);
        return;
    }


    float frameRate = _thumbReader->getProperty<float>("video.frame_rate");
    _time = DS_TimePtr(new DS_Time(frameRate));

    QVariantMap frameInfo = _thumbReader->getProperty<QVariantMap>("video.frame_info.one");

    _fbuf = plugin->getOrCreateFrameBuffer(
        _cacheNode,
        frameInfo,
        true /* force create new framebuffer */ 
    );

    _fbuf->reallocate();

    
    _df.setFileName(_cacheNode->mediaCachePixelDataPath());
    if (_df.open(QIODevice::WriteOnly))
    {
        RLP_LOG_DEBUG("Opened:" << _df.fileName());
    }


    _dfpos.setFileName(_cacheNode->mediaCachePosDataPath());

    if (_dfpos.open(QIODevice::WriteOnly))
    {
        RLP_LOG_DEBUG("Opened:" << _dfpos.fileName());;
    }

}


void
MEDIA_ThumbnailCacheTask::run()
{

    int numFrames = _thumbReader->getProperty<int>("media.frame_count");
    int icount = 0;
    for (int i=_fnum; i != numFrames; ++i)
    {
        if (icount == 50)
        {
            break;
        }

        _time->setVideoFrame(i);
        _thumbReader->readVideo(_time, _fbuf, false);

        int imgWidth = 480;
        QImage fi = _fbuf->toQImage().scaledToWidth(imgWidth, Qt::SmoothTransformation);

        // _cacheNode->thumbCache()->insert(i, fi);

        QByteArray ba;
        QBuffer rbuf(&ba);
        rbuf.open(QIODevice::WriteOnly);
        fi.save(&rbuf, "JPG", 90);
        rbuf.close();

        qint64 imgByteCount = ba.size();

        QVariantMap finfo;
        finfo.insert("pos", _df.pos());
        finfo.insert("size", imgByteCount);
        finfo.insert("w", fi.width());
        finfo.insert("h", fi.height());

        _dfinfo.insert(QString::number(_fnum), finfo);
        _df.write(ba.constData(), imgByteCount);

        QApplication::processEvents();
        icount++;
        _fnum++;
    }

    // RLP_LOG_DEBUG(_fnum << "/" << numFrames)

    if (_fnum == numFrames)
    {
        _cacheNode->setThumbCacheState(MEDIA_CacheNode::THUMB_CACHE_READY);
        RLP_LOG_DEBUG("READY")
        _df.close();

        QString posInfo = UTIL_Convert::serializeToJson(_dfinfo);

        _dfpos.write(QByteArray(posInfo.toUtf8().constData()));
        _dfpos.close();

    }
}