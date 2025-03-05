//
//  Copyright (c) 2014 Justin Ottley. All rights reserved.
//

#ifndef EXTREVLENS_QPSD_MEDIA_VIDEOREADER_H
#define EXTREVLENS_QPSD_MEDIA_VIDEOREADER_H

#include "RlpExtrevlens/RLQPSD_MEDIA/Global.h"

#include "RlpRevlens/DS/Node.h"


class EXTREVLENS_RLQPSD_MEDIA_API RLQPSD_MEDIA_VideoReader : public DS_Node {

public:
    RLP_DEFINE_LOGGER

    RLQPSD_MEDIA_VideoReader(QVariantMap* properties);

    void readVideo(DS_TimePtr timeInfo, DS_BufferPtr destFrame, bool postUpdate = true);
    void scheduleReadTask(DS_TimePtr timeInfo, DS_FrameBufferPtr destFrame, DS_Node::NodeDataType, bool optional = false);
    
    
};

#endif

