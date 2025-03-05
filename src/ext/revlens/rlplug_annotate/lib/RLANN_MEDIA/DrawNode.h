//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef EXTREVLENS_RLANN_MEDIA_NODE_H
#define EXTREVLENS_RLANN_MEDIA_NODE_H

#include "RlpExtrevlens/RLANN_MEDIA/Global.h"

#include "RlpRevlens/DS/Node.h"

#include "RlpExtrevlens/RLANN_DS/DrawControllerBase.h"


class EXTREVLENS_RLANN_MEDIA_API RLANN_MEDIA_Node : public DS_Node {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    RLANN_MEDIA_Node(RLANN_DS_DrawControllerBase* drawController, QVariantMap* properties, DS_NodePtr nodeIn);
    
    void readVideo(DS_TimePtr timeInfo, DS_BufferPtr destFrame, bool postUpdate = true);
    
    void readAudio(DS_TimePtr timeInfo, DS_BufferPtr destBuffer);
    
    void scheduleReadTask(DS_TimePtr timeInfo,
                          DS_BufferPtr destFrame,
                          DS_Node::NodeDataType dataType,
                          bool optional = false);
    
    void scheduleReleaseTask(qlonglong frameNum);

private:
    
    RLANN_DS_DrawControllerBase* _drawController;
};


class EXTREVLENS_RLANN_MEDIA_API RLANN_MEDIA_VideoReadTask : public QRunnable {
    
public:
    
    RLANN_MEDIA_VideoReadTask(RLANN_MEDIA_Node* node,
                              DS_TimePtr timeInfo,
                              DS_BufferPtr destFrame);

    void run();

private:
    
    RLANN_MEDIA_Node* _node;
    DS_TimePtr _timeInfo;
    DS_BufferPtr _destFrame;
};


#endif
