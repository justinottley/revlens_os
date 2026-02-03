
#include "RlpExtrevlens/RLQTMEDIA/NetworkReadHandler.h"
#include "RlpExtrevlens/RLQTMEDIA/NetworkReader.h"
#include "RlpExtrevlens/RLQTMEDIA/DeferredLoadPlugin.h"
#include "RlpExtrevlens/RLQTMEDIA/NetworkPool.h"

RLP_SETUP_LOGGER(ext, RLQTMEDIA, NetworkReadHandler)


RLQTMEDIA_NetworkReadHandler::RLQTMEDIA_NetworkReadHandler()
{
    _lock = new QMutex();

    _client = new CoreNet_WebSocketClient();
    connect(
        _client,
        &CoreNet_WebSocketClient::clientConnected,
        this,
        &RLQTMEDIA_NetworkReadHandler::onClientConnected
    );

    connect(
        _client,
        &CoreNet_WebSocketClient::messageReceived,
        this,
        &RLQTMEDIA_NetworkReadHandler::onMessageReceived
    );

    connect(
        _client,
        &CoreNet_WebSocketClient::binaryReceived,
        this,
        &RLQTMEDIA_NetworkReadHandler::onBinaryReceived
    );

    // QString url = srcInfo.value("media.remote_url").toString();


    // QString url = "ws://192.168.1.66:8140";
    // RLP_LOG_DEBUG("Attempting connection:" << url)

    // _client->setUrl(url);
}


void
RLQTMEDIA_NetworkReadHandler::registerNode(RLQTMEDIA_NetworkReader* reader)
{
    //QMutexLocker l(_lock);
    
    QString nodeUuid = reader->graph()->uuid().toString();

    RLP_LOG_DEBUG(reader << nodeUuid)

    RLQTMEDIA_DeferredLoadPlugin* plugin = reader->getPropertiesPtr()->value("media.plugin.loader").value<RLQTMEDIA_DeferredLoadPlugin*>();
    _nodeMap.insert(nodeUuid, reader);
}


void
RLQTMEDIA_NetworkReadHandler::registerReadRequest(QString path, DS_Node* node, DS_TimePtr timeInfo, DS_BufferPtr destFrame)
{
    //QMutexLocker l(_lock);

    if (!_pathMap.contains(path))
    {
        RLP_LOG_DEBUG("Registering new frame request map:" << path)
        FrameRequestMap* m = new FrameRequestMap();
        _pathMap.insert(path, m);
    }

    FrameRequestMap* m = _pathMap.value(path);
    RLQTMEDIA_NetworkRequestInfo nri(node, timeInfo, destFrame);
    m->insert(timeInfo->mediaVideoFrame(), nri);

    // RLP_LOG_DEBUG(timeInfo->mediaVideoFrame())
}


void
RLQTMEDIA_NetworkReadHandler::requestFrameRead(QString path, RLQTMEDIA_NetworkReader* reader, DS_TimePtr timeInfo, DS_BufferPtr destFrame)
{
    // QString threadName = QString("%1").arg((uint64_t)QThread::currentThread(), 16 );
    // RLP_LOG_DEBUG(threadName)
    
    //QMutexLocker l(_lock);


    if (!_client->isConnected())
    {
        RLP_LOG_ERROR("Client not connected")
        return;
    }

    QString nodeUuid = reader->graph()->uuid().toString();

    // RLP_LOG_DEBUG(reader << nodeUuid)

    if (!_nodeMap.contains(nodeUuid))
    {
        RLP_LOG_DEBUG("Registering Node" << reader << nodeUuid)

        RLQTMEDIA_DeferredLoadPlugin* plugin = reader->getPropertiesPtr()->value("media.plugin.loader").value<RLQTMEDIA_DeferredLoadPlugin*>();

        _nodeMap.insert(nodeUuid, reader);
    }

    qlonglong frameNum = timeInfo->mediaVideoFrame();
    // RLP_LOG_DEBUG(path << frameNum)

    if (!_pathMap.contains(path))
    {
        RLP_LOG_DEBUG("Registering new frame request map:" << path)
        FrameRequestMap* m = new FrameRequestMap();
        _pathMap.insert(path, m);
    }

    FrameRequestMap* m = _pathMap.value(path);
    RLQTMEDIA_NetworkRequestInfo nri(reader, timeInfo, destFrame);
    m->insert(timeInfo->mediaVideoFrame(), nri);
    // m->insert(0, nri);


    QVariantList args;
    args.append(path);
    args.append(frameNum);
    args.append(destFrame->getDataBytes()); // expected buffer size

    _client->sendCall("media.request_frame", args);
    //QApplication::processEvents();
}


void
RLQTMEDIA_NetworkReadHandler::onClientConnected()
{
    RLP_LOG_DEBUG("OK")
}


void
RLQTMEDIA_NetworkReadHandler::requestLoad(QString path, QString nodeUuid)
{
    RLP_LOG_DEBUG("")

    QVariantMap minfo;
    minfo.insert("media.video", path);
    minfo.insert("node.uuid", nodeUuid);

    QVariantList args;
    args.append(minfo);

    _client->sendCall("media.load", args);
}


void
RLQTMEDIA_NetworkReadHandler::onMessageReceived(QVariantMap msgInfo)
{
    QString fromMethod = msgInfo.value("from_method").toString();
    if (fromMethod == "media.load")
    {
        // _lock->lock();

        // update node properties for lookahead
        QVariantMap result = msgInfo.value("result").toMap();
        QVariantMap rproperties = result.value("properties").toMap();
        QVariantMap frameInfo = rproperties.value("video.frame_info.one").toMap();
        QString nodeUuid = rproperties.value("node.uuid").toString();

        if (!_nodeMap.contains(nodeUuid))
        {
            RLP_LOG_ERROR("Cant use this load, node not registered:" << nodeUuid)
            return;
        }

        DS_Node* node = _nodeMap.value(nodeUuid);

        RLQTMEDIA_DeferredLoadPlugin* pl = node->getPropertiesPtr()->value("media.plugin.loader").value<RLQTMEDIA_DeferredLoadPlugin*>();
        pl->setFrameInfo(frameInfo);

        node->getPropertiesPtr()->insert(rproperties);

        // _lock->unlock();

        RLP_LOG_DEBUG("Frame info set:" << nodeUuid)
        // LOG_Logging::pprint(rproperties);
        // RLP_LOG_DEBUG(rproperties)

        QVariantMap data;
        QVariant nv;
        nv.setValue(node);
        data.insert("node", nv);
        
        RLP_LOG_DEBUG("Emitting video source ready")
        emit videoSourceReady(data);
        // node->graph()->emitDataReady("video_source", data);
    }

    else if (fromMethod == "media.request_frame") // DEPRECATED, using binary path instead
    {
        QVariantMap frameInfo = msgInfo.value("result").toMap();

        QString readPath = frameInfo.value("read_path").toString();
        qlonglong frameNum = frameInfo.value("frame_num").toLongLong();
        qlonglong frameSize = frameInfo.value("size").toLongLong();
        QString frameDataStr = frameInfo.value("data").toString();
        int chanOrder = frameInfo.value("chan_order").toInt();

        // RLP_LOG_DEBUG(readPath << frameNum)

        if (_pathMap.contains(readPath))
        {
            FrameRequestMap* frm = _pathMap.value(readPath);
            if (!frm->contains(frameNum))
            {
                RLP_LOG_ERROR("cant use this frame:" << readPath << frameNum)
                return;
            }
            // RLP_LOG_DEBUG(readPath << "got:" << frameNum << "size:" << frameSize)

            RLQTMEDIA_NetworkRequestInfo nri = frm->value(frameNum);
            // nri.destFrame->setDataBytes(frameSize);

            if (nri.destFrame->getDataBytes() == frameSize)
            {
                QByteArray ba = QByteArray::fromBase64(frameDataStr.toLocal8Bit());
                
                QImage i = QImage::fromData(ba, "JPG");
                RLP_LOG_DEBUG("Frame OK, recovered size:" << i.width())
                
                dynamic_cast<DS_FrameBuffer*>(nri.destFrame.get())
                    ->setChannelOrder((DS_FrameBuffer::ChannelOrder)chanOrder);

                memcpy(nri.destFrame->data(), i.bits(), frameSize);

                QApplication::postEvent(
                    nri.destFrame->getOwner(),
                    new MEDIA_ReadEvent(nri.timeInfo,
                                        nri.destFrame,
                                        MEDIA_FrameReadEventType));

                frm->remove(frameNum);

                // nri.destFrame->setData
            } else
            {
                RLP_LOG_ERROR("Framebuffer size mismatch:" << nri.destFrame->getDataBytes()  << "incoming:" << frameSize)
            }

        } else
        {
            RLP_LOG_ERROR("Cant use this frame:" << readPath << ":" << frameNum)
        }
        
    }
}


void
RLQTMEDIA_NetworkReadHandler::onBinaryReceived(const QByteArray& data)
{
    // RLP_LOG_DEBUG("")

    bool headerDone = false;

    int partIdx = 0;
    int idx = 0;

    // QString path;
    //QString frameStr;
    // QString dataSizeStr;
    QString chanOrderStr;

    // QString mdStr;
    QByteArray mdb;

    while (!headerDone)
    {
        char bchar = data[idx];
        if (bchar == '|')
        {
            headerDone = true;

            // partIdx++;
            // if (partIdx == 4)
            //     headerDone = true;

        } else
        {
            mdb += bchar;

            /*
            if (partIdx == 0)
                path += bchar;

            else if (partIdx == 1)
                frameStr += bchar;

            else if (partIdx == 2)
                dataSizeStr += bchar;

            else if (partIdx == 3)
                chanOrderStr += bchar;
            */
        }
        idx++;
    }

    // QString msg = QByteArray::fromBase64(mdb); // .toLocal8Bit());
    QJsonDocument resultData = QJsonDocument::fromJson(QByteArray::fromBase64(mdb));
    QVariantMap md = resultData.object().toVariantMap();

    QString path = md.value("p").toString();
    qlonglong frameNum = md.value("f").toLongLong();
    int channelOrder = md.value("co").toInt();

    // RLP_LOG_DEBUG(md)

    // QString threadName = QString("%1").arg((uint64_t)QThread::currentThread(), 16 );
    // RLP_LOG_DEBUG(threadName << path)
    // RLP_LOG_DEBUG(frameStr << threadName)
    if (_pathMap.contains(path))
    {
        FrameRequestMap* frm = _pathMap.value(path);
        if (!frm->contains(frameNum)) // if (!frm->contains(0))
        {
            RLP_LOG_ERROR("cant use this frame:" << path << frameNum)
            return;
        }

        RLQTMEDIA_NetworkRequestInfo nri = frm->value(frameNum); // (0);

        if (md.contains("fi"))
        {
            QVariantMap frameInfo = md.value("fi").toMap();
            RLP_LOG_WARN("Frame info sent, resizing buffer")

            RLQTMEDIA_DeferredLoadPlugin* pl = nri.node->getPropertiesPtr()->value("media.plugin.loader").value<RLQTMEDIA_DeferredLoadPlugin*>();
            pl->setFrameInfo(frameInfo);

            nri.node->getPropertiesPtr()->insert("video.frame_info.one", frameInfo);


            RLP_LOG_DEBUG("Frame info set:" << frameNum);

            nri.destFrame->reallocate(frameInfo);

            QVariantMap data;
            QVariant nv;
            nv.setValue(nri.node);
            data.insert("node", nv);
            
            RLP_LOG_DEBUG("Emitting video source ready")
            // emit videoSourceReady(data);
            nri.node->graph()->emitDataReady("video_source", data);

        }
        // else // if (nri.destFrame->getDataBytes() == frameSize)
        {

            QByteArray fdata = data.sliced(idx);
            
            RLQTMEDIA_NetworkDecodeTask* ndt = new RLQTMEDIA_NetworkDecodeTask(
                fdata,
                nri,
                channelOrder
            );
            RLQTMEDIA_NetworkPool::instance()->decodePool()->start(ndt);

            // DS_QImageFrameBuffer* qfbuf = dynamic_cast<DS_QImageFrameBuffer*>(nri.destFrame.get());

            // // qfbuf->setChannelOrder((DS_FrameBuffer::ChannelOrder)chanOrder);
            // // qfbuf->getQImage()->loadFromData(fdata, "JPG");

            // QImage i = QImage::fromData(fdata, "JPG");
            // i = i.scaledToWidth(
            //     i.width() * 2,
            //     Qt::SmoothTransformation
            // );
            // qfbuf->getQImage()->swap(i);

            // RLP_LOG_DEBUG(threadName << "Frame OK, recovered size:" << i.width())


            frm->remove(frameNum);
            // RLP_LOG_DEBUG("OK:" << frameNum)

            // RLP_LOG_DEBUG("Requesting next frame:" << (frameNum + 1))

            // Request next frame
            //
            // QVariantList args;
            // args.append(path);
            // args.append(frameNum + 1);
            // args.append(nri.destFrame->getDataBytes()); // expected buffer size

            // _client->sendCall("media.request_frame", args);

        }
        // else
        // {
        //     RLP_LOG_ERROR("FRAME SIZE MISMATCH:" << nri.destFrame->getDataBytes() << "vs" << frameSize)
        // }
    }
}


void
RLQTMEDIA_NetworkReadHandler::handleTask(RLQTMEDIA_NetworkRunnable* task)
{
    // QString threadName = QString("%1").arg((uint64_t)QThread::currentThread(), 16 );
    // RLP_LOG_DEBUG(threadName)

    task->run(this);
}