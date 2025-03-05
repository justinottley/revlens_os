
#include "RlpRevlens/CNTRL/MediaServiceController.h"

#include "RlpRevlens/CNTRL/MainController.h"
#include "RlpRevlens/CNTRL/Server.h"

#include "RlpRevlens/MEDIA/Factory.h"
#include "RlpRevlens/MEDIA/Plugin.h"

#include "RlpRevlens/DS/FrameBuffer.h"

RLP_SETUP_LOGGER(revlens, CNTRL, MediaServiceController)

CNTRL_MediaServiceController::CNTRL_MediaServiceController(
    CNTRL_MainController* cntrl
):
    _cntrl(cntrl)
{
    connect(
        _cntrl,
        &CNTRL_MainController::remoteCall,
        this,
        &CNTRL_MediaServiceController::onRemoteCallReceived
    );
}


QVariantMap
CNTRL_MediaServiceController::runMediaLoad(QVariantMap mediaInfo)
{
    RLP_LOG_DEBUG(mediaInfo)

    QString path = mediaInfo.value("media.video").toString();
    QString inodeGraphUuid = mediaInfo.value("node.uuid").toString();

    QVariantMap rresult;
    rresult.insert("load_path", path);

    if (_nodeMap.contains(path))
    {
        DS_NodePtr node = _nodeMap.value(path);
        QVariantMap properties;
        properties.insert("video.frame_info.one", node->getPropertiesPtr()->value("video.frame_info.one"));
        properties.insert("video.frame_rate", node->getPropertiesPtr()->value("video.frame_rate"));
        properties.insert("node.uuid", inodeGraphUuid);

        rresult.insert("properties", properties);
        rresult.insert("status", "OK");

        return rresult;
    }

    DS_Node::NodeDataType dataType = DS_Node::DATA_VIDEO;

    MEDIA_Factory* factory = MEDIA_Factory::instance();
    QVariantMap result = factory->identifyMedia2(mediaInfo, dataType);

    QVariantMap* rr = new QVariantMap();
    rr->insert(result);

    DS_NodePtr node = nullptr;
    if (dataType == DS_Node::DATA_VIDEO)
    {
        node = factory->createVideoReader(rr);

    } else if (dataType == DS_Node::DATA_AUDIO)
    {
        node = factory->createAudioReader(rr);
    }

    if (node != nullptr)
    {
        node->getPropertiesPtr()->insert("session.frame", 1);
        // Wrap via in-memory cache node

        RLP_LOG_DEBUG("Saving node for" << path)

        _nodeMap.insert(path, node);

        QVariantMap result;
        rresult.insert("status", "OK");

        QVariantMap properties;
        properties.insert("video.frame_info.one", node->getPropertiesPtr()->value("video.frame_info.one"));
        properties.insert("video.frame_rate", node->getPropertiesPtr()->value("video.frame_rate"));
        properties.insert("node.uuid", inodeGraphUuid);

        rresult.insert("properties", properties);
    }
    else
    {
        rresult.insert("status", "err");
        rresult.insert("err_msg", "Load Failed");
    }

    return rresult;
}


QVariantMap
CNTRL_MediaServiceController::runReadFrame(QString path, qlonglong frame)
{
    QVariantMap rresult;
    rresult.insert("read_path", path);
    if (!_nodeMap.contains(path))
    {
        QVariantMap minfo;
        minfo.insert("media.video", path);

        runMediaLoad(minfo);
    }

    if (!_nodeMap.contains(path))
    {
        QString errMsg = QString("Media not loaded: %1").arg(path);
        RLP_LOG_ERROR(errMsg)

        rresult.insert("status", "err");
        rresult.insert("err_msg", errMsg);
        return rresult;
    }


    DS_NodePtr node = _nodeMap.value(path);

    QVariantMap frameInfo = node->getProperty<QVariantMap>("video.frame_info.one");
    DS_TimePtr timeInfo = node->makeTime(frame);

    MEDIA_Plugin* plugin = node->getProperty<MEDIA_Plugin*>("video.plugin");
    DS_FrameBufferPtr destFrame = plugin->getOrCreateFrameBuffer(this, frameInfo);
    destFrame->reallocate();

    node->readVideo(timeInfo, destFrame, false);

    // RLP_LOG_DEBUG(frame << "Read size:" << destFrame->getDataBytes());

    QByteArray ba = QByteArray::fromRawData((char*)destFrame->data(), destFrame->getDataBytes());


    // QString baStr = QString(ba.toBase64());
    QString mdata = QString("%1|%2|%3|%4|")
        .arg(path)
        .arg(frame)
        .arg(destFrame->getDataBytes())
        .arg((int)destFrame->getChannelOrder());

    ba.prepend(mdata.toUtf8());

    rresult.insert("frame_num", frame);
    rresult.insert("size", destFrame->getDataBytes());
    rresult.insert("width", destFrame->getWidth());
    rresult.insert("height", destFrame->getHeight());
    rresult.insert("chan_order", (int)destFrame->getChannelOrder());
    rresult.insert("data", ba);

    
    return rresult;

}


void
CNTRL_MediaServiceController::onRemoteCallReceived(QVariantMap msgObj)
{
    // LOG_Logging::pprint(msgObj);

    QString method = msgObj.value("method").toString();
    QString clientIdent = msgObj.value("ident").toString();
    
    QVariantMap resulte;
    resulte.insert("from_method", method);
    resulte.insert("ident", clientIdent);

    if (method == "media.load")
    {
        QVariantList args = msgObj.value("args").toList();
        if (args.size() == 0)
        {
            RLP_LOG_ERROR("Invalid args");
            return;
        }
        QVariantMap mediaInfo = args.at(0).toMap();

        QVariantMap loadResult = runMediaLoad(mediaInfo);
        resulte.insert("result", loadResult);

        _cntrl->getServerManager()->sendCallToClient(resulte);
    }

    else if (method == "media.request_frame")
    {
        QVariantList args = msgObj.value("args").toList();
        if (args.size() != 2)
        {
            RLP_LOG_ERROR("Invalid args");
            return;
        }

        QString path = args.at(0).toString();
        qlonglong frameNum = args.at(1).toLongLong();

        QVariantMap loadResult = runReadFrame(path, frameNum);
        const QByteArray ba = loadResult.value("data").toByteArray();

        loadResult.remove("data");
        resulte.insert("result", loadResult);

        _cntrl->getServerManager()->sendBinaryToClient(clientIdent, ba);
        // _cntrl->getServerManager()->sendCallToClient(resulte);
    }

    
}


