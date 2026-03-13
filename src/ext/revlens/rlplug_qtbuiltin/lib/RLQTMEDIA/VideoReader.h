//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef EXTREVLENS_RLQTMEDIA_VIDEOREADER_H
#define EXTREVLENS_RLQTMEDIA_VIDEOREADER_H

#include "RlpExtrevlens/RLQTMEDIA/Global.h"

#include "RlpCore/SEQ/Sequence.h"
#include "RlpRevlens/DS/Node.h"

#include "RlpEdb/MEDIA/Controller.h"

class EXTREVLENS_RLQTMEDIA_API RLQTMEDIA_VideoReader : public DS_Node {

public:
    RLP_DEFINE_LOGGER

    RLQTMEDIA_VideoReader(QVariantMap* properties);

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

