
#include "RlpExtrevlens/RLQTMEDIA/NetworkReader.h"
#include "RlpExtrevlens/RLQTMEDIA/DeferredLoadPlugin.h"
#include "RlpExtrevlens/RLQTMEDIA/NetworkReadHandler.h"
#include "RlpExtrevlens/RLQTMEDIA/NetworkPool.h"

#include "RlpRevlens/CNTRL/MainController.h"

#include "RlpRevlens/MEDIA/ReadEvent.h"

RLP_SETUP_LOGGER(ext, RLQTMEDIA, NetworkReader)


RLQTMEDIA_NetworkReader::RLQTMEDIA_NetworkReader(
    QVariantMap* properties, DS_Node::NodeDataType dataType
):
    DS_Node("RLQTMEDIA_NetworkReader", properties),
    _dataType(dataType),
    _started(false)
{
    RLP_LOG_DEBUG("")

    _srcInfo = properties->value("media.source_info").toMap();
    _srcPath = _srcInfo.value("media.video").toString();

    //RLQTMEDIA_NetworkReadManager* nmgr = RLQTMEDIA_NetworkPool::instance()->nmgr();

    //nmgr->requestLoad(_srcPath, this);

    // RLQTMEDIA_NetworkRegisterNodeTask* rnTask = new RLQTMEDIA_NetworkRegisterNodeTask(this);
    // RLQTMEDIA_NetworkPool::instance()->tpool()->start(rnTask);
}


void
RLQTMEDIA_NetworkReader::readVideo(DS_TimePtr timeInfo, DS_BufferPtr destFrame, bool postUpdate)
{
    // if (_started)
    // {
    //     return;
    // }

    // timeInfo->setVideoFrame(1);

    RLP_LOG_DEBUG(timeInfo->videoFrame())

    RLQTMEDIA_NetworkReadTask* readTask = new RLQTMEDIA_NetworkReadTask(_srcPath, this, timeInfo, destFrame);
    RLQTMEDIA_NetworkPool::instance()->start(readTask);

    // _started = true;

    QApplication::postEvent(
        destFrame->getOwner(),
        new MEDIA_ReadEvent(timeInfo,
                            destFrame,
                            MEDIA_FrameReadEventType));
}


void
RLQTMEDIA_NetworkReader::scheduleReadTask(DS_TimePtr timeInfo,
                                    DS_BufferPtr destFrame,
                                    DS_Node::NodeDataType dataType,
                                    bool optional)
{
    // if (_started)
    // {
    //     return;
    // }

    // QString threadName = QString("%1").arg((uint64_t)QThread::currentThread(), 16 );
    // RLP_LOG_DEBUG(threadName)


    RLQTMEDIA_NetworkReadTask* readTask = new RLQTMEDIA_NetworkReadTask(_srcPath, this, timeInfo, destFrame);
    RLQTMEDIA_NetworkPool::instance()->start(readTask);

    // _started = true;
}
