#ifndef REVLENS_MEDIA_READ_EVENT_H
#define REVLENS_MEDIA_READ_EVENT_H

#include "RlpRevlens/MEDIA/Global.h"

#include "RlpRevlens/DS/Time.h"
#include "RlpRevlens/DS/Buffer.h"

static const int MEDIA_FrameReadEventType = QEvent::User + 10;
static const int MEDIA_AudioReadEventType = QEvent::User + 11;

class MEDIA_ReadEvent : public QEvent {
    
public:
    MEDIA_ReadEvent(DS_TimePtr timeInfo, int nodeIndex, DS_BufferPtr frameBuffer, int eventNum) :
        QEvent(QEvent::Type(eventNum)),
        _timeInfo(timeInfo),
        _nodeIdx(nodeIndex),
        _frameBuffer(frameBuffer)
    {
    }
    
    DS_TimePtr getTimeInfo() { return _timeInfo; }
    DS_BufferPtr getFrameBuffer() { return _frameBuffer; }
    int getNodeIndex() { return _nodeIdx; }
    
protected:
    
    DS_TimePtr _timeInfo;
    DS_BufferPtr _frameBuffer;
    int _nodeIdx;

};

#endif