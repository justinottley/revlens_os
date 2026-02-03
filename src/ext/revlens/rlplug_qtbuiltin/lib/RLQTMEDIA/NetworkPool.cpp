
#include "RlpExtrevlens/RLQTMEDIA/NetworkPool.h"
#include "RlpExtrevlens/RLQTMEDIA/NetworkReader.h"
#include "RlpExtrevlens/RLQTMEDIA/DeferredLoadPlugin.h"


RLP_SETUP_LOGGER(ext, RLQTMEDIA, NetworkReadTask)
RLP_SETUP_LOGGER(ext, RLQTMEDIA, NetworkThread)
RLP_SETUP_LOGGER(ext, RLQTMEDIA, NetworkPool)
RLP_SETUP_LOGGER(ext, RLQTMEDIA, NetworkDecodeTask)

RLQTMEDIA_NetworkPool* RLQTMEDIA_NetworkPool::_INSTANCE = nullptr;


RLQTMEDIA_NetworkPool::RLQTMEDIA_NetworkPool()
{
    _tpool = new QThreadPool();
    _tpool->setMaxThreadCount(2);

    RLQTMEDIA_NetworkThread* t = new RLQTMEDIA_NetworkThread(this);
    t->setPriority(QThread::TimeCriticalPriority);
    t->start();

    _t = t;
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
RLQTMEDIA_NetworkPool::start(RLQTMEDIA_NetworkRunnable* task)
{
    // RLP_LOG_DEBUG("")
    emit requestTask(task);
}


// ------------


RLQTMEDIA_NetworkReadTask::RLQTMEDIA_NetworkReadTask(
    QString path,
    RLQTMEDIA_NetworkReader* reader,
    DS_TimePtr timeInfo,
    DS_BufferPtr destFrame
):
    _path(path),
    _reader(reader),
    _timeInfo(timeInfo),
    _destFrame(destFrame)
{
}


void
RLQTMEDIA_NetworkReadTask::run()
{
    // QString threadName = QString("%1").arg((uint64_t)QThread::currentThread(), 16 );

    // RLP_LOG_DEBUG(threadName)
    _handler->requestFrameRead(_path, _reader, _timeInfo, _destFrame);
}


// ----------


RLQTMEDIA_NetworkThread::RLQTMEDIA_NetworkThread(RLQTMEDIA_NetworkPool* pool):
    QThread(),
    _pool(pool),
    _handler(nullptr),
    _connected(false)
{
}


void
RLQTMEDIA_NetworkThread::run()
{
    RLP_LOG_DEBUG("")

    QString threadName = QString("%1").arg((uint64_t)QThread::currentThread(), 16 );
    RLP_LOG_DEBUG("initializing thread-local network handler for thread:" << threadName)

    // connect(
    //     _pool,
    //     &RLQTMEDIA_NetworkPool::requestTask,
    //     this,
    //     &RLQTMEDIA_NetworkThread::runTask,
    //     Qt::QueuedConnection
    // );

    _handler = new RLQTMEDIA_NetworkReadHandler();
    connect(
        _handler->client(),
        &CoreNet_WebSocketClient::clientConnected,
        this,
        &RLQTMEDIA_NetworkThread::onClientConnected,
        Qt::DirectConnection
    );

    connect(
        _pool,
        &RLQTMEDIA_NetworkPool::requestTask,
        _handler,
        &RLQTMEDIA_NetworkReadHandler::handleTask,
        Qt::AutoConnection
    );

    QString url = "ws://192.168.1.66:8140";
    RLP_LOG_DEBUG("Attempting connection:" << url)
    _handler->setUrl(url);

    exec();
}


void
RLQTMEDIA_NetworkThread::onClientConnected()
{
    RLP_LOG_DEBUG("")
    _connected = true;
}


// -------------


RLQTMEDIA_NetworkDecodeTask::RLQTMEDIA_NetworkDecodeTask(
    QByteArray frameData,
    RLQTMEDIA_NetworkRequestInfo nri,
    int channelOrder
):
    _frameData(frameData),
    _nri(nri),
    _channelOrder(channelOrder)
{
}


void
RLQTMEDIA_NetworkDecodeTask::run()
{
    // RLP_LOG_DEBUG("")

    DS_QImageFrameBuffer* qfbuf = dynamic_cast<DS_QImageFrameBuffer*>(_nri.destFrame.get());
    qfbuf->setChannelOrder((DS_FrameBuffer::ChannelOrder)_channelOrder);
    // qfbuf->getQImage()->loadFromData(_frameData, "JPG");

    QImage i = QImage::fromData(_frameData, "JPG");
    // i = i.scaledToWidth(
    //     i.width() * 2,
    //     Qt::SmoothTransformation
    // );
    qfbuf->getQImage()->swap(i);

    QApplication::postEvent(
        _nri.destFrame->getOwner(),
        new MEDIA_ReadEvent(
            _nri.timeInfo,
            _nri.destFrame,
            MEDIA_FrameReadEventType
        )
    );

}