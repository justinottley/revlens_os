//
//  Copyright (c) 2014 Justin Ottley. All rights reserved.
//

#ifndef EXTREVLENS_RLEXRMEDIA_VIDEOREADER_H
#define EXTREVLENS_RLEXRMEDIA_VIDEOREADER_H

#include "RlpExtrevlens/RLEXRMEDIA/Global.h"

#include "RlpCore/SEQ/Sequence.h"
#include "RlpRevlens/DS/Node.h"




class EXTREVLENS_RLEXRMEDIA_API RLEXRMEDIA_VideoReader : public DS_Node {
    
public:
    RLP_DEFINE_LOGGER

    RLEXRMEDIA_VideoReader(QVariantMap* properties);
    
    void readVideo(
        DS_TimePtr timeInfo,
        DS_BufferPtr destFrame,
        bool postUpdate = true);
    
    
    void scheduleReadTask(
        DS_TimePtr timeInfo,
        DS_BufferPtr destFrame,
        DS_Node::NodeDataType dataType,
        bool optional = false);
    
private:
    SEQ_SequencePtr _seq;

};

#endif

