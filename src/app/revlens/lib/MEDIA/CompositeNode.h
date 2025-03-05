//
//  Copyright (c) 2014 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_MEDIA_COMPOSITENODE_H
#define REVLENS_MEDIA_COMPOSITENODE_H

#include "RlpRevlens/MEDIA/Global.h"

#include "RlpRevlens/DS/Time.h"
#include "RlpRevlens/DS/Buffer.h"
#include "RlpRevlens/DS/FrameBuffer.h"
#include "RlpRevlens/DS/AudioBuffer.h"
#include "RlpRevlens/DS/Node.h"
#include "RlpRevlens/MEDIA/ReadEvent.h"


class REVLENS_MEDIA_API MEDIA_CompositeNode : public DS_Node {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    MEDIA_CompositeNode(QVariantMap* properties, DS_NodePtr nodeIn);

    void readVideo(DS_TimePtr timeInfo, DS_BufferPtr destFrame, bool postUpdate = true);

    void readAudio(DS_TimePtr timeInfo, DS_BufferPtr destBuffer);

    void scheduleReadTask(DS_TimePtr timeInfo,
                          DS_BufferPtr destFrame,
                          DS_Node::NodeDataType dataType,
                          bool optional = false);

    bool event(QEvent* event);


public slots:
    void addInput(DS_NodePtr node);

};


class REVLENS_MEDIA_API MEDIA_CompositeReadTask : public QRunnable {

public:
    MEDIA_CompositeReadTask(
        MEDIA_CompositeNode* reader,
        DS_TimePtr timeInfo,
        DS_BufferPtr destFrame
    );

    void run();

private:
    MEDIA_CompositeNode* _reader;
    DS_TimePtr _timeInfo;
    DS_BufferPtr _destFrame;
};

#endif
