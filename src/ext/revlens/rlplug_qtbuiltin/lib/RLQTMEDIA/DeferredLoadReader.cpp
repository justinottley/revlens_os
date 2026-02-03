
#include "RlpExtrevlens/RLQTMEDIA/DeferredLoadReader.h"
#include "RlpExtrevlens/RLQTMEDIA/DeferredLoadPlugin.h"
#include "RlpExtrevlens/RLQTMEDIA/DeferredTasks.h"

#include "RlpRevlens/MEDIA/ReadEvent.h"
#include "RlpRevlens/MEDIA/Factory.h"

#include "RlpRevlens/CNTRL/MainController.h"
#include "RlpRevlens/CNTRL/MediaManager.h"
#include "RlpRevlens/CNTRL/Server.h"


RLP_SETUP_LOGGER(ext, RLQTMEDIA, DeferredLoadReader)

RLQTMEDIA_DeferredLoadReader::RLQTMEDIA_DeferredLoadReader(QVariantMap* properties, DS_Node::NodeDataType dataType):
    DS_Node("RLQTMEDIA_DeferredLoadReader", properties),
    _dataType(dataType),
    _videoNode(nullptr),
    _audioNode(nullptr)
{

    QVariant nWrapper;
    nWrapper.setValue(this);
    properties->insert("graph.node.deferred_load", nWrapper);

    connect(
        this,
        &RLQTMEDIA_DeferredLoadReader::requestSiteClientCall,
        CNTRL_MainController::instance()->getServerManager(),
        &CNTRL_Server::siteClientSendCall,
        Qt::AutoConnection
    );

    // For UI responsiveness, but may not be necessary?
    QTimer::singleShot(1, this, &RLQTMEDIA_DeferredLoadReader::loadDeferred);
}


void
RLQTMEDIA_DeferredLoadReader::loadDeferred()
{
    // RLP_LOG_DEBUG((int)_dataType)

    if (!_properties->contains("media.source_info"))
    {
        RLP_LOG_ERROR("Source info not found, cannot load!")
        return;
    }

    QVariantMap srcInfo = _properties->value("media.source_info").toMap();
    // RLP_LOG_DEBUG(srcInfo)

    QRunnable* task = new RLQTMEDIA_DeferredLoadTask(this, srcInfo, _dataType);

    RLQTMEDIA_DeferredLoadPlugin* pl = _properties->value("media.plugin.loader").value<RLQTMEDIA_DeferredLoadPlugin*>();
    pl->getReadThreadPool(DS_Node::DATA_LOAD)->start(task);
}


void
RLQTMEDIA_DeferredLoadReader::loadMedia(QVariantMap srcInfo)
{
    RLP_LOG_DEBUG(srcInfo)

    _videoNode = nullptr;
    _audioNode = nullptr;

    _properties->insert("media.source_info", srcInfo);
    QTimer::singleShot(1, this, &RLQTMEDIA_DeferredLoadReader::loadDeferred);
}


void
RLQTMEDIA_DeferredLoadReader::setVideoInputNode(DS_NodePtr node)
{
    // RLP_LOG_DEBUG("");

    if (node == nullptr) return;

    QVariantMap* props = node->getPropertiesPtr();
    MEDIA_Plugin* srcPlugin = props->value("video.plugin").value<MEDIA_Plugin*>();

    // RLP_LOG_DEBUG("Got video plugin from source:" << srcPlugin)

    qlonglong incomingFrameCount = props->value("media.frame_count").toLongLong();
    qlonglong myFrameCount = _properties->value("media.frame_count").toLongLong();

    if (incomingFrameCount != myFrameCount)
    {
        RLP_LOG_WARN("FRAME COUNT CHANGING:" << myFrameCount << "->" << incomingFrameCount)
        _properties->insert("media.frame_count", incomingFrameCount);
    }

    // LOG_Logging::pprint(*props);

    // setup deferred load plugin to redirect video/audio thread pool to source media thread pool
    RLQTMEDIA_DeferredLoadPlugin* dplugin = _properties->value("media.plugin.loader").value<RLQTMEDIA_DeferredLoadPlugin *>();
    dplugin->setupReadThreadPool(DS_Node::DATA_VIDEO, srcPlugin);


    QVariant v;
    v.setValue(srcPlugin);

    // Update plugin for lookahead

    QStringList keyList = props->keys();
    QStringList::iterator it;

    for (it = keyList.begin(); it != keyList.end(); ++it)
    {
        QString keyName = (*it);
        if (keyName.startsWith("video."))
        {
            _properties->insert(keyName, props->value(keyName));
        }
    }

    /*
    // Update frame info for lookahead
    _properties->insert("video.plugin", v);
    _properties->insert("video.plugin.src", v);
    _properties->insert("video.frame_info.one", props->value("video.frame_info.one"));
    _properties->insert("video.frame_info", props->value("video.frame_info"));
    _properties->insert("video.frame_rate", props->value("video.frame_rate"));
    */

    _videoNode = node;

    QVariantMap data;

    QVariant nv;
    nv.setValue(this);
    data.insert("node", nv);

    QVariant rv;
    rv.setValue(node);
    data.insert("reader", rv);

    emitDataReadyToGraph("video_source", data);

}


void
RLQTMEDIA_DeferredLoadReader::setAudioInputNode(DS_NodePtr node)
{
    if (node == nullptr) return;

    // RLP_LOG_DEBUG("")

    QVariantMap* props = node->getPropertiesPtr();


    // Bring forward the real path to the audio into this node
    _properties->insert("media.audio", props->value("media.audio"));


    // Bring forward all the audio details into this node
    // if (props->contains("audio.plugin"))
    {
        QList<QString> propKeys = props->keys();
        for (int pi=0; pi != propKeys.size(); ++pi)
        {
            QString propKey = propKeys.at(pi);
            if (propKey == "audio.plugin")
            {
                continue;
                RLP_LOG_DEBUG("skipping audio.plugin")
            }

            if (propKey.startsWith("audio."))
            {
                // RLP_LOG_DEBUG("Attempting insert:" << propKey)
                _properties->insert(propKey, props->value(propKey));
            }
        }
    }

    // Send the video frame rate into the properties of the source node
    //
    QVariantMap* gprops = _properties->value("graph").value<DS_Graph*>()->propertiesPtr();
    if (gprops->contains("video.frame_rate"))
    {
        props->insert("video.frame_rate", gprops->value("video.frame_rate"));
    }

    // Done syncing properties, stash source audio node
    //
    _audioNode = node;


    // Emit that source audio node is ready
    //
    QVariantMap data;

    QVariant v;
    v.setValue(this);
    data.insert("node", v);

    graph()->emitDataReady("audio_source", data);
}


void
RLQTMEDIA_DeferredLoadReader::readVideo(DS_TimePtr timeInfo, DS_BufferPtr destFrame, bool postUpdate)
{
    // RLP_LOG_DEBUG(timeInfo->videoFrame())

    if (_videoNode == nullptr)
    {
        DS_QImageFrameBuffer* fbuf = dynamic_cast<DS_QImageFrameBuffer*>(destFrame.get());

        QImage* img = fbuf->getQImage();
        img->fill(Qt::black);
        QPainter p(img);
        QFont f = p.font();
        f.setPointSize(100);
        p.setFont(f);
        p.setPen(QColor(100, 100, 100));
        p.drawText(100, img->height() / 2, "Loading..");

        if (postUpdate)
        {
            QApplication::postEvent(destFrame->getOwner(),
                                    new MEDIA_ReadEvent(timeInfo,
                                                        destFrame,
                                                        MEDIA_FrameReadEventType));
        }

    } else
    {
        _videoNode->readVideo(timeInfo, destFrame, postUpdate);
    }
}



void
RLQTMEDIA_DeferredLoadReader::readAudio(DS_TimePtr timeInfo, DS_BufferPtr destBuffer)
{
    // RLP_LOG_DEBUG(timeInfo->mediaVideoFrame())

    if (_audioNode == nullptr)
    {
        destBuffer->getBufferMetadata()->insert("audio.read_result", DS_Buffer::READ_NONE);
    } else
    {
        _audioNode->readAudio(timeInfo, destBuffer);
    }
}


void
RLQTMEDIA_DeferredLoadReader::scheduleReadTask(
    DS_TimePtr timeInfo,
    DS_BufferPtr destFrame,
    DS_Node::NodeDataType dataType,
    bool optional)
{
    QRunnable* task = new RLQTMEDIA_DeferredLoadReadTask(this, timeInfo, destFrame);

    if (optional)
    {
        _properties->value("media.plugin.loader").
            value<RLQTMEDIA_DeferredLoadPlugin *>()->
            getReadThreadPool(dataType)->
            tryStart(task);

    } else
    {
        _properties->value("media.plugin.loader").
            value<RLQTMEDIA_DeferredLoadPlugin *>()->
            getReadThreadPool(dataType)->
            start(task);
    }

}


// --------------

