//
//  Copyright (c) 2014 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_MEDIA_READER_H
#define REVLENS_MEDIA_READER_H

#include "RlpRevlens/MEDIA/Global.h"

#include "RlpRevlens/DS/FrameBuffer.h"
#include "RlpRevlens/DS/Node.h"


class REVLENS_MEDIA_API MEDIA_AVInputNode : public DS_Node {
    
public:
    MEDIA_AVInputNode(QVariantMap* properties, DS_NodePtr videoIn, DS_NodePtr audioIn) :
        DS_Node(properties)
    {
        _inputs.push_back(videoIn);
        _inputs.push_back(audioIn);
    }
    
    
    void readVideo(long frameNum, DS_FrameBufferPtr destFrame, bool postUpdate = true) {
        return _inputs[0]->readVideo(frameNum, destFrame, postUpdate);
    }
    
    void readAudio() {
        return _inputs[1].readAudio();
    }
    
};

#endif
