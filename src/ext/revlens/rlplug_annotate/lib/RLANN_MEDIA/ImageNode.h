//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef EXTREVLENS_RLANN_MEDIA_NODE_H
#define EXTREVLENS_RLANN_MEDIA_NODE_H

#include "RlpExtrevlens/RLANN_MEDIA/Global.h"

#include "RlpRevlens/DS/Node.h"

#include "RlpExtrevlens/RLANN_DS/DrawControllerBase.h"


class EXTREVLENS_RLANN_MEDIA_API RLANN_MEDIA_ImageNode : public DS_Node {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER


public:
    RLANN_MEDIA_ImageNode(QVariantMap* properties);

    void readVideo(DS_TimePtr timeInfo, DS_BufferPtr destFrame, bool postUpdate = true);

    void scheduleReadTask(DS_TimePtr timeInfo,
                          DS_BufferPtr destFrame,
                          DS_Node::NodeDataType dataType,
                          bool optional = false);


private:
    QImage _img;
};


class EXTREVLENS_RLANN_MEDIA_API RLANN_MEDIA_ImageReadTask : public QRunnable {
    
public:
    
    RLANN_MEDIA_ImageReadTask(RLANN_MEDIA_ImageNode* node,
                              DS_TimePtr timeInfo,
                              DS_BufferPtr destFrame);

    void run();

private:
    
    RLANN_MEDIA_ImageNode* _node;
    DS_TimePtr _timeInfo;
    DS_BufferPtr _destFrame;
};


#endif
