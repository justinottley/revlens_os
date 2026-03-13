
#ifndef EXTREVLENS_RLQTDS_ONTHEGONODE_H
#define EXTREVLENS_RLQTDS_ONTHEGONODE_H

#include "RlpExtrevlens/RLQTDS/Global.h"

#include "RlpRevlens/DS/Node.h"


class RLQTDS_OnTheGoNode : public DS_Node {
    Q_OBJECT

public:
    RLQTDS_OnTheGoNode(DS_NodePtr baseNode);

    void readVideo(DS_TimePtr timeInfo, DS_BufferPtr destFrame, bool postUpdate = true);

    void scheduleReadTask(
        DS_TimePtr timeInfo,
        DS_BufferPtr destBuffer,
        DS_Node::NodeDataType dataType,
        bool optional = false);

private:
    DS_NodePtr _baseNode;


};

#endif
