
#include "RlpCore/NET/WebSocketIOHandler.h"


RLP_SETUP_LOGGER(core, CoreNet, FileIO)
RLP_SETUP_LOGGER(core, CoreNet, WebSocketIOHandler)



CoreNet_FileIO::CoreNet_FileIO(QString path, qint64 chunkSize):
    _path(path),
    _chunkSize(chunkSize),
    _handle(-1),
    _pos(-1),
    _qfile(path)
{
    if (_qfile.open(QIODevice::ReadOnly)) {
        _handle = _qfile.handle();
        _pos = 0;

        RLP_LOG_DEBUG(path << "OK")
    } else {
        RLP_LOG_ERROR(path << "open failed")
    }
}


bool
CoreNet_FileIO::check(QString path)
{
    QFile f(path);
    bool result = f.open(QIODevice::ReadOnly);
    f.close();

    return result;
}


QVariantMap
CoreNet_FileIO::next(int pos)
{
    if (pos != _pos)
    {
        RLP_LOG_DEBUG("Seeking to" << pos)
        _qfile.seek(pos);
    }

    QByteArray rdata = _qfile.read(_chunkSize);
    RLP_LOG_DEBUG(rdata.size())

    QString data = QString(rdata.toBase64());

    _pos = _qfile.pos();

    QVariantMap result;
    result.insert("data", data);
    result.insert("size", rdata.size());
    result.insert("pos", _pos);
    result.insert("fh", _handle);

    return result;
}


CoreNet_WebSocketIOHandler::CoreNet_WebSocketIOHandler()
{
    RLP_LOG_DEBUG("")
}


QVariantMap
CoreNet_WebSocketIOHandler::handleDownloadStart(QString path)
{
    RLP_LOG_DEBUG(path)

    QVariantMap result;

    if (CoreNet_FileIO::check(path))
    {
        CoreNet_FileIO* fio = new CoreNet_FileIO(path);
        _fileMap.insert(fio->fileHandle(), fio);

        result.insert("fh", fio->fileHandle());
        result.insert("pos", fio->filePos());
        result.insert("size_total", fio->fileSize());

    } else {

        result.insert("fh", -1);
        result.insert("pos", -1);
    
    }

    return result;
}


QVariantMap
CoreNet_WebSocketIOHandler::handleDownloadChunk(QVariantMap fileInfo)
{
    RLP_LOG_DEBUG(fileInfo)

    int fhandle = fileInfo.value("fh").toInt();
    if (_fileMap.contains(fhandle))
    {
        int pos = fileInfo.value("pos").toInt();
        CoreNet_FileIO* fio = _fileMap.value(fhandle);
        QVariantMap data = fio->next(pos);

        return data;
    } else {
        
        QVariantMap res;
        res.insert("err_msg", "invalid file handle");
        return res;
    }

    QVariantMap res;
    res.insert("err_msg", "Error");

    return res;
}


bool
CoreNet_WebSocketIOHandler::handleDownloadEnd(QVariantMap fileInfo)
{
    int fhandle = fileInfo.value("fh").toInt();
    if (_fileMap.contains(fhandle))
    {
        CoreNet_FileIO* fio = _fileMap.value(fhandle);
        fio->close();

        _fileMap.remove(fhandle);

        return true;
    }

    return false;
}


QVariantMap
CoreNet_WebSocketIOHandler::handleDownload(QVariantMap msgObj)
{
    RLP_LOG_DEBUG(msgObj)

    QString methodName = msgObj.value("method").toString();
    QString ident = msgObj.value("ident").toString();

    QVariantMap sessionObj;

    QString status = "OK";
    QString errMsg = "";

    QString runId = "";
    if (msgObj.contains("session"))
    {
        sessionObj = msgObj.value("session").toMap();
        if (sessionObj.contains("run_id"))
        {
            runId = sessionObj.value("run_id").toString();
        }
    }

    sessionObj.insert("client.ident", ident);

    QVariantMap rresult;
    rresult.insert("status", "OK");
    rresult.insert("ident", ident);
    rresult.insert("run_id", runId);

    if (methodName == "download_start")
    {
        QVariantList margs = msgObj.value("args").toList();
        // QVariantMap kwargs = msgObj.value("kwargs").toMap();

        QString path = margs.at(0).toString();
        if (msgObj.contains("store_path"))
        {
            path = msgObj.value("store_path").toString();
        }

        QVariantMap result = handleDownloadStart(path);
        rresult.insert("result", result);

    } else if (methodName == "download_chunk")
    {
        QVariantMap kwargs = msgObj.value("kwargs").toMap();
        QVariantMap data = handleDownloadChunk(kwargs);

        rresult.insert("result", data);

    } else if (methodName == "download_end")
    {
        QVariantMap kwargs = msgObj.value("kwargs").toMap();
        bool result = handleDownloadEnd(kwargs);

        rresult.insert("result", result);

    } else {

        rresult.insert("err_msg", "Invalid method");
    }

    return rresult;
}