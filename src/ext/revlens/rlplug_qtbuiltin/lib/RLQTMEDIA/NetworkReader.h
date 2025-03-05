
#ifndef EXTREVLENS_RLQTMEDIA_NETWORK_READER_H
#define EXTREVLENS_RLQTMEDIA_NETWORK_READER_H


#include "RlpExtrevlens/RLQTMEDIA/Global.h"
#include "RlpExtrevlens/RLQTMEDIA/NetworkReadHandler.h"

#include "RlpRevlens/DS/Node.h"

#include "RlpCore/NET/WebSocketClient.h"


class EXTREVLENS_RLQTMEDIA_API RLQTMEDIA_NetworkReader : public DS_Node {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    RLQTMEDIA_NetworkReader(QVariantMap* properties, DS_Node::NodeDataType dataType);

    void readVideo(
        DS_TimePtr timeInfo,
        DS_BufferPtr destFrame,
        bool postUpdate = true);

private:
    DS_Node::NodeDataType _dataType;

    QVariantMap _srcInfo;
    QString _srcPath;

};

#endif
