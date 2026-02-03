
#ifndef EXTREVLENS_RLQTMEDIA_NETWORK_READ_HANDLER_H
#define EXTREVLENS_RLQTMEDIA_NETWORK_READ_HANDLER_H


#include "RlpExtrevlens/RLQTMEDIA/Global.h"

#include "RlpRevlens/DS/Node.h"

#include "RlpCore/NET/WebSocketClient.h"

#include "RlpRevlens/MEDIA/ReadEvent.h"


class RLQTMEDIA_DeferredLoadPlugin;
class RLQTMEDIA_NetworkReader;

class RLQTMEDIA_NetworkReadHandler;

class RLQTMEDIA_NetworkRunnable : public QRunnable {

public:
    void run(RLQTMEDIA_NetworkReadHandler* handler)
    {
        _handler = handler;
        run();
    }

    virtual void run() {}

protected:
    RLQTMEDIA_NetworkReadHandler* _handler;

};



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



class RLQTMEDIA_NetworkReadHandler : public QObject {
    Q_OBJECT

signals:
    void videoSourceReady(QVariantMap info);

public:
    RLP_DEFINE_LOGGER

    RLQTMEDIA_NetworkReadHandler();

    void registerReadRequest(QString path, DS_Node* node, DS_TimePtr timeInfo, DS_BufferPtr destFrame);
    void registerNode(RLQTMEDIA_NetworkReader* reader);

    CoreNet_WebSocketClient* client() { return _client; }

public slots:

    // void requestFrame(QString path, DS_TimePtr timeInfo, DS_BufferPtr destFrame);
    // void requestFrameRead(QString path, qlonglong frameNum);
    void requestFrameRead(QString path, RLQTMEDIA_NetworkReader* node, DS_TimePtr timeInfo, DS_BufferPtr destFrame);    
    void requestLoad(QString path, QString nodeUuid);

    void setUrl(QString url) { _client->setUrl(url); }
    void onClientConnected();
    
    // void onFrameRequested(QVariantList args);
    void onMessageReceived(QVariantMap msgInfo);
    void onBinaryReceived(const QByteArray& data);

    void handleTask(RLQTMEDIA_NetworkRunnable* task);

private:
    QMutex* _lock;
    // QVariantMap _srcInfo;
    // int _nodeIndex;
    // DS_NodePtr _node;

    CoreNet_WebSocketClient* _client;

    QMap<QString, FrameRequestMap*> _pathMap;
    QMap<QString, DS_Node*> _nodeMap;

};



#endif
