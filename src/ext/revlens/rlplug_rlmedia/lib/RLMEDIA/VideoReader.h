//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef EXTREVLENS_RLMEDIA_VIDEOREADER_H
#define EXTREVLENS_RLMEDIA_VIDEOREADER_H

#include "RlpExtrevlens/RLMEDIA/Global.h"

#include "RlpRevlens/DS/Node.h"
#include "RlpRevlens/DS/QImageFrameBuffer.h"

#include "RlpEdb/MEDIA/Controller.h"


class EXTREVLENS_RLMEDIA_API RLMEDIA_VideoReader : public DS_Node {

public:
    RLP_DEFINE_LOGGER

    RLMEDIA_VideoReader(QVariantMap* properties);

    void readVideo(
        DS_TimePtr timeInfo,
        DS_BufferPtr destFrame,
        bool postUpdate = true);

    void scheduleReadTask(
        DS_TimePtr timeInfo,
        DS_BufferPtr destFrame,
        DS_Node::NodeDataType dataType,
        bool optional = false);

private: // methods
    void _readVideo(qlonglong frameNum, DS_QImageFrameBuffer* fbuf);

private: // members
    QString _videoCodec;
    QThreadStorage<EdbMedia_Controller*> _mediaStorage;

};

#endif

