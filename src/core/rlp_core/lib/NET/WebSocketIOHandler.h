#ifndef CORE_NET_WEBSOCKET_IO_HANDLER_H
#define CORE_NET_WEBSOCKET_IO_HANDLER_H

#include "RlpCore/NET/Global.h"


class CORE_NET_API CoreNet_FileIO {

public:
    RLP_DEFINE_LOGGER

    CoreNet_FileIO(QString path, qint64 chunkSize=32768);

    int fileHandle() { return _handle; }
    int filePos() { return _pos; }
    qint64 fileSize() { return _qfile.size(); }
    QString filePath() { return _path; }
    QVariantMap next(int pos);

    void close() { _qfile.close(); }

    static bool check(QString path);


private:
    QString _path;
    qint64 _chunkSize;
    int _handle;
    qint64 _pos;
    QFile _qfile;
    

};


class CORE_NET_API CoreNet_WebSocketIOHandler : public QObject {
    Q_OBJECT

signals:
    void dataReady(QVariantMap data);

public:
    RLP_DEFINE_LOGGER

    CoreNet_WebSocketIOHandler();

    QVariantMap handleDownloadStart(QString path);
    QVariantMap handleDownloadChunk(QVariantMap fileInfo);
    bool handleDownloadEnd(QVariantMap fileInfo);
    QVariantMap handleDownload(QVariantMap msgObj);

private:
    QMap<int, CoreNet_FileIO*> _fileMap;

};

#endif
