
#ifndef EXTREVLENS_RLQTMEDIA_DEFERRED_TASKS_H
#define EXTREVLENS_RLQTMEDIA_DEFERRED_TASKS_H

#include "RlpExtrevlens/RLQTMEDIA/Global.h"

#include "RlpCore/NET/RPCFuture.h"
#include "RlpRevlens/DS/Node.h"

class RLQTMEDIA_DeferredLoadReader;

class RLQTMEDIA_DeferredLoadLocateCallback : public CoreNet_RPCFuture {

public:
    RLP_DEFINE_LOGGER

    RLQTMEDIA_DeferredLoadLocateCallback(
        RLQTMEDIA_DeferredLoadReader* reader,
        DS_Node::NodeDataType dataType);

    void onResultReady(QVariantMap result);

private:
    RLQTMEDIA_DeferredLoadReader* _reader;
    DS_Node::NodeDataType _dataType;

};

class RLQTMEDIA_DeferredLoadIdentifyTask : public QRunnable {

public:
    RLP_DEFINE_LOGGER

    RLQTMEDIA_DeferredLoadIdentifyTask(
        RLQTMEDIA_DeferredLoadReader* reader,
        QVariantMap srcInfo,
        DS_Node::NodeDataType dataType);

    void run();

private:
    RLQTMEDIA_DeferredLoadReader* _reader;
    QVariantMap _srcInfo;
    DS_Node::NodeDataType _dataType;

};

class EXTREVLENS_RLQTMEDIA_API RLQTMEDIA_DeferredLoadTask : public QRunnable {

public:
    RLP_DEFINE_LOGGER

    RLQTMEDIA_DeferredLoadTask(
        RLQTMEDIA_DeferredLoadReader* reader,
        QVariantMap srcInfo,
        DS_Node::NodeDataType dataType);

    void run();

private:

    void initAudio(QVariantMap* nodeProps);
    void runLocal();
    void runRemote();

private:

    RLQTMEDIA_DeferredLoadReader* _reader;
    QVariantMap _srcInfo;
    DS_Node::NodeDataType _dataType;

};


// --------


class EXTREVLENS_RLQTMEDIA_API RLQTMEDIA_DeferredLoadReadTask : public QRunnable {

public:
    RLP_DEFINE_LOGGER

    RLQTMEDIA_DeferredLoadReadTask(
        RLQTMEDIA_DeferredLoadReader* reader,
        DS_TimePtr timeInfo,
        DS_BufferPtr destFrame
    );

    void run();

private:

    RLQTMEDIA_DeferredLoadReader* _reader;
    DS_TimePtr _timeInfo;
    DS_BufferPtr _destFrame;
};


#endif
