//
//  Copyright (c) 2014 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_MEDIA_AVINPUTNODE_H
#define REVLENS_MEDIA_AVINPUTNODE_H

#include "RlpRevlens/MEDIA/Global.h"

#include "RlpRevlens/DS/Time.h"
#include "RlpRevlens/DS/Buffer.h"
#include "RlpRevlens/DS/FrameBuffer.h"
#include "RlpRevlens/DS/AudioBuffer.h"
#include "RlpRevlens/DS/Node.h"


class REVLENS_MEDIA_API MEDIA_AVInputNode : public DS_Node {
    Q_OBJECT

public:
    enum InputIndex {
        VIDEO_IN,
        AUDIO_IN
    };

public:
    RLP_DEFINE_LOGGER

    MEDIA_AVInputNode(QVariantMap* properties, DS_NodePtr videoIn, DS_NodePtr audioIn);

    void readVideo(DS_TimePtr timeInfo, DS_BufferPtr destFrame, bool postUpdate = true);

    void readAudio(DS_TimePtr timeInfo, DS_BufferPtr destBuffer);

    void scheduleReadTask(DS_TimePtr timeInfo,
                          DS_BufferPtr destFrame,
                          DS_Node::NodeDataType dataType,
                          bool optional = false);
    
private:
    
    DS_AudioBufferPtr _emptyAudioBuffer;
    
};

#endif
