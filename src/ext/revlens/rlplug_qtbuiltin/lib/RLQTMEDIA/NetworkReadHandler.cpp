
#include "RlpExtrevlens/RLQTMEDIA/NetworkReadHandler.h"
#include "RlpExtrevlens/RLQTMEDIA/DeferredLoadPlugin.h"



RLP_SETUP_LOGGER(ext, RLQTMEDIA, NetworkReadHandler)
RLP_SETUP_LOGGER(ext, RLQTMEDIA, NetworkReadManager)

RLQTMEDIA_NetworkPool* RLQTMEDIA_NetworkPool::_INSTANCE = nullptr;


RLQTMEDIA_NetworkReadHandler::RLQTMEDIA_NetworkReadHandler()
{
    _lock = new QMutex();

    _client = new CoreNet_WebSocketClient();
    connect(
        _client,
        &CoreNet_WebSocketClient::onClientConnected,
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

    // RLP_LOG_DEBUG("Attempting connection:" << url)
    QString url = "ws://127.0.0.1:8140";
    _client->setUrl(url);
}


void
RLQTMEDIA_NetworkReadHandler::registerLoadRequest(DS_Node* node)
{
    QMutexLocker l(_lock);

    QString nodeUuid = node->graph()->uuid().toString();

    RLP_LOG_DEBUG(nodeUuid)

    _nodeMap.insert(nodeUuid, node);
}


void
RLQTMEDIA_NetworkReadHandler::registerReadRequest(QString path, DS_Node* node, DS_TimePtr timeInfo, DS_BufferPtr destFrame)
{
    QMutexLocker l(_lock);

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
RLQTMEDIA_NetworkReadHandler::requestFrameRead(QString path, qlonglong frameNum)
{
    QMutexLocker l(_lock);

    // qlonglong frame = timeInfo->mediaVideoFrame();
    // RLP_LOG_DEBUG(path << frameNum)

    if (!_client->isConnected())
    {
        RLP_LOG_ERROR("Client not connected")
        return;
    }

    if (!_pathMap.contains(path))
    {
        RLP_LOG_ERROR("media path request not registered:" << path)
        return;
    }


    QVariantList args;
    args.append(path);
    args.append(frameNum);

    _client->sendCall("media.request_frame", args);
}


void
RLQTMEDIA_NetworkReadHandler::onClientConnected()
{
    RLP_LOG_DEBUG("")
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

        RLP_LOG_DEBUG("Frame info set:" << nodeUuid)
        // RLP_LOG_DEBUG(result)
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
                // RLP_LOG_DEBUG("Frame OK, recovered size:" << ba.size())

                dynamic_cast<DS_FrameBuffer*>(nri.destFrame.get())
                    ->setChannelOrder((DS_FrameBuffer::ChannelOrder)chanOrder);

                memcpy(nri.destFrame->data(), ba.data(), frameSize);

                QApplication::postEvent(
                    nri.destFrame->getOwner(),
                    new MEDIA_ReadEvent(nri.timeInfo,
                                        nri.node->getProperty<int>("index"),
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
    bool headerDone = false;

    int partIdx = 0;
    int idx = 0;

    QString path;
    QString frameStr;
    QString dataSizeStr;
    QString chanOrderStr;

    while (!headerDone)
    {
        char bchar = data[idx];
        if (bchar == '|')
        {
            partIdx++;
            if (partIdx == 4)
                headerDone = true;

        } else
        {
            if (partIdx == 0)
                path += bchar;

            else if (partIdx == 1)
                frameStr += bchar;

            else if (partIdx == 2)
                dataSizeStr += bchar;

            else if (partIdx == 3)
                chanOrderStr += bchar;
        }
        idx++;
    }

    if (_pathMap.contains(path))
    {
        FrameRequestMap* frm = _pathMap.value(path);
        qlonglong frameNum = frameStr.toLongLong();
        int frameSize = dataSizeStr.toInt();
        int chanOrder = chanOrderStr.toInt();

        if (!frm->contains(frameNum))
        {
            RLP_LOG_ERROR("cant use this frame:" << path << frameNum)
            return;
        }

        RLQTMEDIA_NetworkRequestInfo nri = frm->value(frameNum);

        if (nri.destFrame->getDataBytes() == frameSize)
        {

            QByteArray fdata = data.sliced(idx);
            dynamic_cast<DS_FrameBuffer*>(nri.destFrame.get())
                ->setChannelOrder((DS_FrameBuffer::ChannelOrder)chanOrder);

            memcpy(nri.destFrame->data(), fdata.data(), frameSize);

            QApplication::postEvent(
                nri.destFrame->getOwner(),
                new MEDIA_ReadEvent(nri.timeInfo,
                                    nri.node->getProperty<int>("index"),
                                    nri.destFrame,
                                    MEDIA_FrameReadEventType));

            frm->remove(frameNum);
        }
    }
}


RLQTMEDIA_NetworkReadManager::RLQTMEDIA_NetworkReadManager():
    _hindex(0)
{
    _lock = new QMutex();
}


void
RLQTMEDIA_NetworkReadManager::init()
{
    QMutexLocker l(_lock);

    RLP_LOG_DEBUG("")

    // Only create a new handler if this is actually a new thread we're running in
    if (!_storage.hasLocalData())
    {
        QString threadName = QString("%1").arg((uint64_t)QThread::currentThread(), 16 );

        RLP_LOG_DEBUG("initializing thread-local network handler for thread:" << threadName)
        RLQTMEDIA_NetworkReadHandler* h = new RLQTMEDIA_NetworkReadHandler();
        _storage.setLocalData(h);

        _handlers.append(h);
    }
}


void
RLQTMEDIA_NetworkReadManager::requestLoad(QString path, DS_Node* node)
{
    QMutexLocker l(_lock);

    for (int i=0; i != _handlers.size(); ++i)
    {
        RLQTMEDIA_NetworkReadHandler* h = _handlers.at(i);
        h->registerLoadRequest(node);

        QString nodeUuid = node->graph()->uuid().toString();

        QMetaObject::invokeMethod(
            h,
            "requestLoad",
            Qt::AutoConnection,
            Q_ARG(QString, path),
            Q_ARG(QString, nodeUuid)
        );
    }

}
void
RLQTMEDIA_NetworkReadManager::requestFrame(QString path, DS_Node* node, DS_TimePtr timeInfo, DS_BufferPtr destFrame)
{
    QMutexLocker l(_lock);

    // RLP_LOG_DEBUG(path << frameNum)

    if (_handlers.size() == 0)
    {
        RLP_LOG_ERROR("No handlers, aborting" << path << timeInfo->mediaVideoFrame())
        return;
    }

    qlonglong frameNum = timeInfo->mediaVideoFrame();
    RLQTMEDIA_NetworkReadHandler* h = _handlers.at(_hindex);

    h->registerReadRequest(path, node, timeInfo, destFrame);

    QMetaObject::invokeMethod(
        h,
        "requestFrameRead",
        Qt::AutoConnection,
        Q_ARG(QString, path),
        Q_ARG(qlonglong, frameNum)
    );

    // Cycle to next read handler
    _hindex += 1;
    if (_hindex == _handlers.size())
        _hindex = 0;
}


RLQTMEDIA_NetworkPool::RLQTMEDIA_NetworkPool()
{
    _networkPool = new QThreadPool();
    _networkPool->setExpiryTimeout(-1);
    _networkPool->setMaxThreadCount(1);

    _nmgr = new RLQTMEDIA_NetworkReadManager();
    QThread* t = new QThread();
    QObject::connect(
        t,
        &QThread::started,
        _nmgr,
        &RLQTMEDIA_NetworkReadManager::init
    );
    t->start();
}


RLQTMEDIA_NetworkPool*
RLQTMEDIA_NetworkPool::instance()
{
    if (RLQTMEDIA_NetworkPool::_INSTANCE == nullptr)
    {
        RLQTMEDIA_NetworkPool::_INSTANCE = new RLQTMEDIA_NetworkPool();
    }

    return RLQTMEDIA_NetworkPool::_INSTANCE;
}

void
RLQTMEDIA_NetworkConnectionTask::run()
{
    RLQTMEDIA_NetworkReadManager* mgr = RLQTMEDIA_NetworkPool::instance()->nmgr();
    mgr->init();
}