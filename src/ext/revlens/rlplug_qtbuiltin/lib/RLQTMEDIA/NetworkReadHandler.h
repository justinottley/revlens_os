
#ifndef EXTREVLENS_RLQTMEDIA_NETWORK_READ_HANDLER_H
#define EXTREVLENS_RLQTMEDIA_NETWORK_READ_HANDLER_H


#include "RlpExtrevlens/RLQTMEDIA/Global.h"

#include "RlpRevlens/DS/Node.h"

#include "RlpCore/NET/WebSocketClient.h"

#include "RlpRevlens/MEDIA/ReadEvent.h"


class RLQTMEDIA_DeferredLoadPlugin;

class RLQTMEDIA_NetworkRequestInfo {

public:
    RLQTMEDIA_NetworkRequestInfo():
        node(nullptr),
        timeInfo(nullptr),
        destFrame(nullptr)
    {
    }

    RLQTMEDIA_NetworkRequestInfo(DS_Node* node, DS_TimePtr timeInfo, DS_BufferPtr destFrame):
        node(node),
        timeInfo(timeInfo),
        destFrame(destFrame)
    {
    }

    DS_Node* node;
    DS_TimePtr timeInfo;
    DS_BufferPtr destFrame;
};

typedef QMap<qlonglong, RLQTMEDIA_NetworkRequestInfo> FrameRequestMap;



class RLQTMEDIA_NetworkReadTask : public QRunnable {

public:
    RLQTMEDIA_NetworkReadTask(DS_TimePtr timeInfo, DS_BufferPtr destFrame):
        _timeInfo(timeInfo),
        _destFrame(destFrame)
    {
    }

    void run();


private:
    DS_TimePtr _timeInfo;
    DS_BufferPtr _destFrame;

};


class RLQTMEDIA_NetworkReadHandler : public QObject {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    RLQTMEDIA_NetworkReadHandler();

    void registerReadRequest(QString path, DS_Node* node, DS_TimePtr timeInfo, DS_BufferPtr destFrame);
    void registerLoadRequest(DS_Node* node);

public slots:

    // void requestFrame(QString path, DS_TimePtr timeInfo, DS_BufferPtr destFrame);
    void requestFrameRead(QString path, qlonglong frameNum);
    void requestLoad(QString path, QString nodeUuid);

    void onClientConnected();
    
    // void onFrameRequested(QVariantList args);
    void onMessageReceived(QVariantMap msgInfo);
    void onBinaryReceived(const QByteArray& data);


private:
    QMutex* _lock;
    // QVariantMap _srcInfo;
    // int _nodeIndex;
    // DS_NodePtr _node;

    CoreNet_WebSocketClient* _client;

    QMap<QString, FrameRequestMap*> _pathMap;
    QMap<QString, DS_Node*> _nodeMap;

};


class RLQTMEDIA_NetworkReadManager : public QObject {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    RLQTMEDIA_NetworkReadManager();

    void requestFrame(QString path, DS_Node* node, DS_TimePtr timeInfo, DS_BufferPtr destFrame);
    void requestLoad(QString path, DS_Node* node);

public slots:
    void init(); // DS_NodePtr node);

private:
    QThreadStorage<RLQTMEDIA_NetworkReadHandler*> _storage;
    QMutex* _lock;
    QList<RLQTMEDIA_NetworkReadHandler*> _handlers;
    int _hindex;

};



class RLQTMEDIA_NetworkPool : public QObject {
    Q_OBJECT

public:
    RLQTMEDIA_NetworkPool(); // shared thread pool for entire app

    static RLQTMEDIA_NetworkPool* instance();

    QThreadPool* tpool() { return _networkPool; }
    RLQTMEDIA_NetworkReadManager* nmgr() { return _nmgr; }

private:
    static RLQTMEDIA_NetworkPool* _INSTANCE;
    QThreadPool* _networkPool;
    RLQTMEDIA_NetworkReadManager* _nmgr;


};


class RLQTMEDIA_NetworkConnectionTask : public QRunnable {

public:
    RLQTMEDIA_NetworkConnectionTask() {}

    void run();

};

#endif
