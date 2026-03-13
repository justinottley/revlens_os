
#include "RlpEdb/MEDIA/Controller.h"

RLP_SETUP_LOGGER(edb, EdbMedia, Controller)

EdbMedia_Controller::EdbMedia_Controller(QString filePath, bool encryptFrames):
    _filePath(filePath),
    _encryptFrames(encryptFrames)
{
    _qcntrl = new EdbCntrl_Query(filePath, "rlmedia", "rlp_media_v1");
    _schema = new EdbMedia_Schema(_qcntrl->mgr());
    _caes = new CRYPT_AES();

    // TODO FIXME: init keys from remote
    if (encryptFrames)
    {
        const char* key = "";
        const char* iv = "";
        _caes->initKey(key, iv);
    }

    // If this is an existing file on disk, initialize known streams
    //
    QVariantList sfilters;
    QStringList returnFields = {"name", "uuid"};
    QVariantList sresult = _qcntrl->find("Stream", sfilters, returnFields);

    // RLP_LOG_DEBUG(sresult)

    for (int i=0; i != sresult.size(); ++i)
    {
        QVariantMap sr = sresult.at(i).toMap();
        QString suuid = sr.value("uuid").toString();
        QString sname = sr.value("name").toString();
        _streamMap.insert(sname, QUuid(suuid));
    }

}


bool
EdbMedia_Controller::isValid()
{
    return _qcntrl->conn()->isValid();
}


QVariantMap
EdbMedia_Controller::getVideoStreamInfo(QString streamName)
{
    QVariantMap result;

    if (!_streamMap.contains(streamName))
    {
        RLP_LOG_ERROR("unknown stream:" << streamName)
        return result;
    }

    QUuid streamUuid = _streamMap.value(streamName);

    QVariantList filters;
    filters.append(QVariant(QVariantList({"uuid", "is", streamUuid})));

    QVariantMap fieldInfo = _qcntrl->getEntityTypeFields("VideoStream");
    QStringList returnList = fieldInfo.keys();

    QVariantList sresult = _qcntrl->find("VideoStream", filters, returnList);

    if (sresult.size() == 1)
    {
        return sresult.at(0).toMap();
    }

    return result;
}


EdbDs_Entity*
EdbMedia_Controller::addVideoStream(QString name, QString uuid)
{
    EdbDs_Entity* vs = new EdbDs_Entity(_schema->VideoStreamType(), name, uuid);
    vs->setFieldValue("stream_type", QString("video"));

    _streamMap.insert(name, QUuid(vs->uuid()));

    return vs;
}


bool
EdbMedia_Controller::writeFrame(QString streamName, qlonglong frameNum, QByteArray data)
{
    if (!_streamMap.contains(streamName))
    {
        RLP_LOG_ERROR("unknown stream:" << streamName)
        return false;
    }

    if (_encryptFrames)
    {
        data = _caes->encryptData(data);
    }

    QUuid streamUuid = _streamMap.value(streamName);

    QString fn = QString("%1_%2").arg(streamName).arg(frameNum);

    EdbDs_Entity frame(_schema->FrameInfoType(), fn, "");
    frame.setFieldValue("stream", streamUuid);
    frame.setFieldValue("frame_num", frameNum);
    frame.setFieldValue("data", data);
    frame.setFieldValue("byte_count", data.size());

    return true;
}


QByteArray
EdbMedia_Controller::getFrameData(QUuid streamUuid, qlonglong frameNum)
{
    QVariantList filters;
    filters.append(QVariant(QVariantList({"stream", "is", streamUuid})));
    filters.append(QVariant(QVariantList({"frame_num", "is", frameNum})));

    QStringList returnFields = {"data"};
    QVariantList fresult = _qcntrl->find("FrameInfo", filters, returnFields);

    RLP_LOG_DEBUG("Got" << fresult.size() << "results")

    if (fresult.size() != 1)
    {
        RLP_LOG_ERROR("invalid result for frame" << frameNum << "got" << fresult.size() << "results")
        return QByteArray();
    }

    QVariantMap m = fresult.at(0).toMap();
    QByteArray result = m.value("data").toByteArray();
    if (_encryptFrames)
    {
        result = _caes->decryptData(result);
    }

    return m.value("data").toByteArray();
}


QByteArray
EdbMedia_Controller::getFrameData(QString streamName, qlonglong frameNum)
{
    if (!_streamMap.contains(streamName))
    {
        RLP_LOG_ERROR("stream not found:" << streamName)
        return QByteArray();
    }

    QUuid streamUuid = _streamMap.value(streamName);

    return getFrameData(streamUuid, frameNum);
}


QVariantList
EdbMedia_Controller::getAllFrameData(QString streamName)
{
    if (!_streamMap.contains(streamName))
    {
        RLP_LOG_ERROR("stream not found:" << streamName)
        return QVariantList();
    }

    QUuid streamUuid = _streamMap.value(streamName);
    QVariantList filters;
    filters.append(QVariant(QVariantList({"stream", "is", streamUuid})));

    QStringList returnFields = {"data", "frame_num"};
    return _qcntrl->find("FrameInfo", filters, returnFields);
}


bool
EdbMedia_Controller::hasFrame(QString streamName, qlonglong frameNum)
{
    if (!_streamMap.contains(streamName))
    {
        RLP_LOG_ERROR("stream not found:" << streamName)
        return false;
    }

    QUuid streamUuid = _streamMap.value(streamName);
    QVariantList filters;
    filters.append(QVariant(QVariantList({"stream", "is", QUuid(streamUuid)})));
    filters.append(QVariant(QVariantList({"frame_num", "is", frameNum})));

    QStringList returnFields = {};
    QVariantList fresult = _qcntrl->find("FrameInfo", filters, returnFields);

    return (fresult.size() == 1);

}