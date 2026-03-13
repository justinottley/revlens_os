//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef EXTREVLENS_RLFFMPEG_BYPASSVIDEOREADER_H
#define EXTREVLENS_RLFFMPEG_BYPASSVIDEOREADER_H

#include "RlpExtrevlens/RLFFMPEG/Global.h"

#include "RlpRevlens/DS/Node.h"


class EXTREVLENS_RLFFMPEG_API RLFFMPEG_BypassVideoReader : public DS_Node {

public:
    RLFFMPEG_BypassVideoReader(QVariantMap* properties);

    void readVideo(
        DS_TimePtr timeInfo,
        DS_BufferPtr destFrame,
        bool postUpdate = true);


    void scheduleReadTask(
        DS_TimePtr timeInfo,
        DS_BufferPtr destFrame,
        DS_Node::NodeDataType dataType,
        bool optional = false);

};


class EXTREVLENS_RLFFMPEG_API RLFFMPEG_BypassVideoReadTask : public QRunnable {
    
public:
    
    RLFFMPEG_BypassVideoReadTask(RLFFMPEG_BypassVideoReader* reader,
                     DS_TimePtr timeInfo,
                     DS_BufferPtr destFrame);
    
    void run();
    
private:
    
    RLFFMPEG_BypassVideoReader* _reader;
    DS_TimePtr _timeInfo;
    DS_BufferPtr _destFrame;
};


#endif

