

#ifndef REVLENS_AUDIO_OUTPUT_DEVICE_H
#define REVLENS_AUDIO_OUTPUT_DEVICE_H

#include "RlpRevlens/AUDIO/Global.h"

#include "RlpRevlens/DS/Session.h"
#include "RlpRevlens/DS/ControllerBase.h"


struct NodeAudioInfo {
    DS_NodePtr node;
    qlonglong frameStart;
    qlonglong frameCount;
    int bytesPerVideoFrame;
    qint64 maxAudioBytes; // based on number of video frames
    qint64 bytePos;
};

class AUDIO_OutputDevice : public QIODevice {
    Q_OBJECT

signals:
    void requestAudioStop(qlonglong frameNum);

public:
    RLP_DEFINE_LOGGER

    AUDIO_OutputDevice(DS_ControllerBase* controller);

    void setTargetFrameRate(float frameRate);
    float getTargetFrameRate() { return _frameRate; }

    void clearAll();
    void updateToFrame(qlonglong frameNum);

    void setScrubbing(qlonglong frameNum, bool scrubbing, bool autoStop=false);
    void setScrubFrame(qlonglong frameNum) { _scrubFrame = frameNum; }

    bool inScrub() { return _scrubbing; }


    qint64 readData(char* data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);


private:
    bool setNextNode();


private:

    DS_ControllerBase* _controller;

    qlonglong _currentFrameNum;
    NodeAudioInfo _currNodeInfo;

    float _frameRate;
    int _bytesPerFrame;

    bool _scrubbing;
    bool _scrubAutoStop;
    qlonglong _scrubFrame;

};

#endif
