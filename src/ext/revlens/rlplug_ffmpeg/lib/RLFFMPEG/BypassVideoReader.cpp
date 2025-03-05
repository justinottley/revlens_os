//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include "RlpExtrevlens/RLFFMPEG/BypassVideoReader.h"
#include "RlpExtrevlens/RLFFMPEG/Plugin.h"

#include "RlpRevlens/MEDIA/Plugin.h"
#include "RlpRevlens/MEDIA/ReadEvent.h"


RLFFMPEG_BypassVideoReader::RLFFMPEG_BypassVideoReader(QVariantMap* properties) :
    DS_Node("RLFFMPEG_BypassVideoReader", properties)
{
}


void
RLFFMPEG_BypassVideoReader::readVideo(DS_TimePtr timeInfo, DS_BufferPtr destFrame, bool postUpdate)
{
    // nothing

    if (postUpdate) {
        QApplication::postEvent(destFrame->getOwner(),
                                new MEDIA_ReadEvent(timeInfo,
                                                    getProperty<int>("index"),
                                                    destFrame,
                                                    MEDIA_FrameReadEventType));
    }
    
}


void
RLFFMPEG_BypassVideoReader::scheduleReadTask(
    DS_TimePtr timeInfo,
    DS_BufferPtr destFrame,
    DS_Node::NodeDataType dataType,
    bool optional)
{
    
    QRunnable* task = new RLFFMPEG_BypassVideoReadTask(this, timeInfo, destFrame);
    
    _properties->value("video.plugin").
                value<RLFFMPEG_Plugin *>()->
                getReadThreadPool(dataType)->
                start(task);
    
}



RLFFMPEG_BypassVideoReadTask::RLFFMPEG_BypassVideoReadTask(RLFFMPEG_BypassVideoReader* reader,
                                           DS_TimePtr timeInfo,
                                           DS_BufferPtr destFrame) :
    _reader(reader),
    _timeInfo(timeInfo),
    _destFrame(destFrame)
{
}


void
RLFFMPEG_BypassVideoReadTask::run()
{
    _reader->readVideo(_timeInfo, _destFrame);
}

