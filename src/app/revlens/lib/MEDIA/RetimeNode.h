//
//  Copyright (c) 2014 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_MEDIA_RETIMENODE_H
#define REVLENS_MEDIA_RETIMENODE_H

#include "RlpRevlens/MEDIA/Global.h"

#include "RlpRevlens/DS/Time.h"
#include "RlpRevlens/DS/Buffer.h"
#include "RlpRevlens/DS/FrameBuffer.h"
#include "RlpRevlens/DS/AudioBuffer.h"
#include "RlpRevlens/DS/Node.h"
#include "RlpRevlens/MEDIA/ReadEvent.h"



class REVLENS_MEDIA_API MEDIA_RetimeNode : public DS_Node {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    MEDIA_RetimeNode(QVariantMap* properties, DS_NodePtr nodeIn);

    void readVideo(DS_TimePtr timeInfo, DS_BufferPtr destFrame, bool postUpdate = true);

    void readAudio(DS_TimePtr timeInfo, DS_BufferPtr destBuffer);

    void scheduleReadTask(DS_TimePtr timeInfo,
                          DS_BufferPtr destFrame,
                          DS_Node::NodeDataType dataType,
                          bool optional = false);

    void retime(DS_TimePtr timeInfo);

    bool event(QEvent* event);

public slots:

    void enable() { _enabled = true; }
    void disable() { _enabled = false; }

    void setInPoint(long inPoint) { _frameIn = inPoint; }
    void setOutPoint(long outPoint) { _frameOut = outPoint; }


protected:
    bool _enabled;
    qlonglong _frameIn;
    qlonglong _frameOut;

};


class REVLENS_MEDIA_API MEDIA_RetimeVideoReadTask : public QRunnable {

public:
    MEDIA_RetimeVideoReadTask(MEDIA_RetimeNode* reader,
                              DS_TimePtr timeInfo,
                              DS_BufferPtr destFrame);

    void run();

private:
    MEDIA_RetimeNode* _node;
    DS_TimePtr _timeInfo;
    DS_BufferPtr _destFrame;

};
#endif
