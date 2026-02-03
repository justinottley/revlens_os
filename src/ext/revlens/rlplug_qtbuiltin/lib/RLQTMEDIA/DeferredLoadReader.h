
#ifndef EXTREVLENS_RLQTMEDIA_DEFERRED_LOAD_READER_H
#define EXTREVLENS_RLQTMEDIA_DEFERRED_LOAD_READER_H

#include "RlpExtrevlens/RLQTMEDIA/Global.h"

#include "RlpCore/NET/RPCFuture.h"

#include "RlpRevlens/DS/Node.h"
#include "RlpRevlens/DS/AudioBuffer.h"

#include "RlpCore/LOG/Logging.h"

class EXTREVLENS_RLQTMEDIA_API RLQTMEDIA_DeferredLoadReader : public DS_Node {
    Q_OBJECT

signals:
    void requestSiteClientCall(
        QString method, QVariantList args, QVariantMap kwargs, QVariantMap skwargs, CoreNet_RPCFuture* callback=nullptr);

public:
    RLP_DEFINE_LOGGER

    RLQTMEDIA_DeferredLoadReader(QVariantMap* properties, DS_Node::NodeDataType dataType);

    void setVideoInputNode(DS_NodePtr node);
    void setAudioInputNode(DS_NodePtr node);

    void readVideo(
        DS_TimePtr timeInfo,
        DS_BufferPtr destFrame,
        bool postUpdate = true);

    void readAudio(
        DS_TimePtr timeInfo,
        DS_BufferPtr destBuffer);

    void scheduleReadTask(
        DS_TimePtr timeInfo,
        DS_BufferPtr destFrame,
        DS_Node::NodeDataType dataType,
        bool optional = false);

    void emitRequestSiteClientCall(
        QString method, QVariantList args, QVariantMap kwargs, QVariantMap skwargs, CoreNet_RPCFuture* callback=nullptr)
    {
        emit requestSiteClientCall(method, args, kwargs, skwargs, callback);
    }

public slots:
    void loadMedia(QVariantMap srcInfo);


private slots:
    void loadDeferred();

private:
    DS_Node::NodeDataType _dataType;
    DS_NodePtr _videoNode;
    DS_NodePtr _audioNode;
};


#endif
