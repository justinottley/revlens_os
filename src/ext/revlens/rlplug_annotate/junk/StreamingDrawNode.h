

#ifndef EXTREVLENS_RLANN_MEDIA_STREAMING_NODE_H
#define EXTREVLENS_RLANN_MEDIA_STREAMING_NODE_H

#include "RlpExtrevlens/RLANN_MEDIA/Global.h"

#include "RlpRevlens/DS/Node.h"

#include "RlpExtrevlens/RLANN_DS/FrameBuffer.h"
#include "RlpExtrevlens/RLANN_DS/DrawControllerBase.h"


class EXTREVLENS_RLANN_MEDIA_API RLANN_MEDIA_StreamingNode : public DS_Node {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    RLANN_MEDIA_StreamingNode(
        RLANN_DS_DrawControllerBase* drawController,
        QVariantMap* properties,
        DS_NodePtr nodeIn
    );

    void readVideo(DS_TimePtr timeInfo, DS_BufferPtr destFrame, bool postUpdate = true);


private:

    RLANN_DS_DrawControllerBase* _drawController;
    RLANN_DS_FrameBufferPtr _fbuf;


};




class EXTREVLENS_RLANN_MEDIA_API RLANN_MEDIA_StreamingVideoReadTask : public QRunnable {

public:

    RLANN_MEDIA_StreamingVideoReadTask(RLANN_MEDIA_StreamingNode* node,
                              DS_TimePtr timeInfo,
                              DS_BufferPtr destFrame);

    void run();

private:

    RLANN_MEDIA_StreamingNode* _node;
    DS_TimePtr _timeInfo;
    DS_BufferPtr _destFrame;
};


#endif
