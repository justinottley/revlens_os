

#ifndef EXTREVLENS_RLQTMEDIA_NETWORK_POOL_H
#define EXTREVLENS_RLQTMEDIA_NETWORK_POOL_H

#include "RlpExtrevlens/RLQTMEDIA/Global.h"
#include "RlpExtrevlens/RLQTMEDIA/NetworkReadHandler.h"

#include "RlpRevlens/DS/Node.h"

class RLQTMEDIA_NetworkReader;
class RLQTMEDIA_NetworkThread;

/*
 * Setup and track a thread pool of network connections
 */
class RLQTMEDIA_NetworkPool : public QObject {
    Q_OBJECT

signals:
    void requestTask(RLQTMEDIA_NetworkRunnable* task);

public:
    RLP_DEFINE_LOGGER

    RLQTMEDIA_NetworkPool(); // shared thread pool for entire app

    static RLQTMEDIA_NetworkPool* instance();

    QThreadPool* decodePool() { return _tpool; }

public slots:

    void start(RLQTMEDIA_NetworkRunnable* task);


private:
    static RLQTMEDIA_NetworkPool* _INSTANCE;

    QThreadPool* _tpool;
    RLQTMEDIA_NetworkThread* _t;
};



class RLQTMEDIA_NetworkThread : public QThread {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    RLQTMEDIA_NetworkThread(RLQTMEDIA_NetworkPool* pool);

public slots:

    void onClientConnected();
    void run();


private:
    RLQTMEDIA_NetworkPool* _pool;
    RLQTMEDIA_NetworkReadHandler* _handler;

    bool _connected;

};


class RLQTMEDIA_NetworkReadTask : public RLQTMEDIA_NetworkRunnable {

public:
    RLP_DEFINE_LOGGER

    RLQTMEDIA_NetworkReadTask(
        QString path,
        RLQTMEDIA_NetworkReader* reader,
        DS_TimePtr timeInfo,
        DS_BufferPtr destFrame
    );

    void run();

private:
    QString _path;
    RLQTMEDIA_NetworkReader* _reader;
    DS_TimePtr _timeInfo;
    DS_BufferPtr _destFrame;
};



class RLQTMEDIA_NetworkDecodeTask : public QRunnable {

public:
    RLP_DEFINE_LOGGER

    RLQTMEDIA_NetworkDecodeTask(
        QByteArray frameData,
        RLQTMEDIA_NetworkRequestInfo nri,
        int channelOrder
    );

    void run();

private:

    QByteArray _frameData;
    RLQTMEDIA_NetworkRequestInfo _nri;
    int _channelOrder;

};

#endif