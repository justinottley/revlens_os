

#include "RlpExtrevlens/RLQTMEDIA/Global.h"
#include "RlpExtrevlens/RLQTMEDIA/DeferredTasks.h"
#include "RlpExtrevlens/RLQTMEDIA/DeferredLoadReader.h"
#include "RlpExtrevlens/RLQTMEDIA/DeferredLoadPlugin.h"

#include "RlpRevlens/CNTRL/MainController.h"
#include "RlpRevlens/CNTRL/Server.h"

#include "RlpRevlens/MEDIA/Factory.h"

#include "RlpCore/UTIL/AppGlobals.h"


RLP_SETUP_LOGGER(ext, RLQTMEDIA, DeferredLoadLocateCallback)
RLP_SETUP_LOGGER(ext, RLQTMEDIA, DeferredLoadIdentifyTask)
RLP_SETUP_LOGGER(ext, RLQTMEDIA, DeferredLoadTask)
RLP_SETUP_LOGGER(ext, RLQTMEDIA, DeferredLoadReadTask)


RLQTMEDIA_DeferredLoadLocateCallback::RLQTMEDIA_DeferredLoadLocateCallback(
    RLQTMEDIA_DeferredLoadReader* reader,
    DS_Node::NodeDataType dataType
):
    CoreNet_RPCFuture(),
    _reader(reader),
    _dataType(dataType)
{
}


void
RLQTMEDIA_DeferredLoadLocateCallback::onResultReady(QVariantMap msgObj)
{
    QVariantMap srcInfo = msgObj.value("result").toMap();

    QVariantMap* ag = UTIL_AppGlobals::instance()->globalsPtr();
    QString url = QString("http://%1:8000").arg(
        ag->value("edb.site_hostname").toString());

    QString vidPath = srcInfo.value("media.video").toString();
    vidPath = QString("%1%2").arg(url, vidPath);

    srcInfo.insert("media.video", vidPath);

    LOG_Logging::pprint(srcInfo);

    QVariantMap* rr = new QVariantMap();
    rr->insert(srcInfo);

    MEDIA_Factory* factory = MEDIA_Factory::instance();

    DS_NodePtr node = nullptr;
    if (_dataType == DS_Node::DATA_VIDEO)
    {
        node = factory->createVideoReader(rr);
        _reader->setVideoInputNode(node);

    } else if (_dataType == DS_Node::DATA_AUDIO)
    {
        node = factory->createAudioReader(rr);
        _reader->setAudioInputNode(node);
    }

    node->getPropertiesPtr()->insert("media.network", true);

    // QRunnable* task = new RLQTMEDIA_DeferredLoadIdentifyTask(_reader, srcInfo, _dataType);
    //
    // QVariantMap* nodeProps = _reader->getPropertiesPtr();
    // nodeProps->value("media.plugin.loader").
    //     value<RLQTMEDIA_DeferredLoadPlugin *>()->
    //     getReadThreadPool(DS_Node::DATA_LOAD)->
    //     start(task);
}


// --------


RLQTMEDIA_DeferredLoadIdentifyTask::RLQTMEDIA_DeferredLoadIdentifyTask(
    RLQTMEDIA_DeferredLoadReader* reader,
    QVariantMap srcInfo,
    DS_Node::NodeDataType dataType
):
    _reader(reader),
    _srcInfo(srcInfo),
    _dataType(dataType)
{
}


void
RLQTMEDIA_DeferredLoadIdentifyTask::run()
{
    RLP_LOG_DEBUG("")

    // QString serverUrl = "http://192.168.1.67:8000";

    QVariantMap* ag = UTIL_AppGlobals::instance()->globalsPtr();
    QString serverUrl = QString("http://%1:8000").arg(
        ag->value("edb.site_hostname").toString());
    // QString serverUrl = "http://172.30.30.227:8000";
    // QString serverUrl = "http://172.30.48.105:8000";

    QVariantMap srcInfo = _srcInfo;

    QString vidUrl = QString("%1%2").arg(serverUrl).arg(srcInfo.value("media.video").toString());
    srcInfo.insert("media.video", vidUrl);

    MEDIA_Factory* factory = MEDIA_Factory::instance();
    QVariantMap result = factory->identifyMedia2(srcInfo, (DS_Node::NodeDataType)_dataType);

    QVariantMap* rr = new QVariantMap();
    rr->insert(result);

    DS_NodePtr node = nullptr;
    if (_dataType == DS_Node::DATA_VIDEO)
    {
        node = factory->createVideoReader(rr);
        _reader->setVideoInputNode(node);

    } else if (_dataType == DS_Node::DATA_AUDIO)
    {
        node = factory->createAudioReader(rr);
        _reader->setAudioInputNode(node);
    }

    node->getPropertiesPtr()->insert("media.network", true);

}

// --------


RLQTMEDIA_DeferredLoadTask::RLQTMEDIA_DeferredLoadTask(
    RLQTMEDIA_DeferredLoadReader* reader,
    QVariantMap srcInfo,
    DS_Node::NodeDataType dataType
):
    _reader(reader),
    _srcInfo(srcInfo),
    _dataType(dataType)
{
}


void
RLQTMEDIA_DeferredLoadTask::runRemote()
{
    // RLP_LOG_DEBUG(_srcInfo)

    if (_dataType != DS_Node::DATA_VIDEO)
    {
        RLP_LOG_WARN("Skipping, not video")
        return;
    }

    _srcInfo.insert("media.video.preferred_proxy", ".mp4");

    QVariantList args;
    args.append(_srcInfo);
    args.append((int)_dataType);

    RLQTMEDIA_DeferredLoadLocateCallback* cb = new RLQTMEDIA_DeferredLoadLocateCallback(_reader, _dataType);

    _reader->emitRequestSiteClientCall(
        "media.locate",
        args,
        QVariantMap(),
        QVariantMap(),
        cb
    );
}


void
RLQTMEDIA_DeferredLoadTask::runLocal()
{
    // RLP_LOG_DEBUG(_srcInfo)

    MEDIA_Factory* factory = MEDIA_Factory::instance();
    QVariantMap result = factory->identifyMedia2(_srcInfo, _dataType);

    QVariantMap* rr = new QVariantMap();
    rr->insert(result);

    DS_NodePtr node = nullptr;
    if (_dataType == DS_Node::DATA_VIDEO)
    {
        node = factory->createVideoReader(rr);
        _reader->setVideoInputNode(node);

    } else if (_dataType == DS_Node::DATA_AUDIO)
    {
        node = factory->createAudioReader(rr);
        _reader->setAudioInputNode(node);
    }
}


void
RLQTMEDIA_DeferredLoadTask::run()
{
    #ifdef SCAFFOLD_IOS
    runRemote();
    #else
    runLocal();
    #endif
}


// --------


RLQTMEDIA_DeferredLoadReadTask::RLQTMEDIA_DeferredLoadReadTask(
    RLQTMEDIA_DeferredLoadReader* reader,
    DS_TimePtr timeInfo,
    DS_BufferPtr destFrame):
        _reader(reader),
        _timeInfo(timeInfo),
        _destFrame(destFrame)
{
}


void
RLQTMEDIA_DeferredLoadReadTask::run()
{
    //RLP_LOG_DEBUG("")
    _reader->readVideo(_timeInfo, _destFrame);
}
