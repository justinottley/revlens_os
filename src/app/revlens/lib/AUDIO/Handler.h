
#ifndef REVLENS_AUDIO_HANDLER_H
#define REVLENS_AUDIO_HANDLER_H

#include "RlpRevlens/AUDIO/Global.h"
#include "RlpRevlens/AUDIO/OutputDevice.h"

#include "RlpRevlens/DS/ControllerBase.h"

#include <QtMultimedia/QAudioFormat>
#include <QtMultimedia/QAudioDevice>
#include <QtMultimedia/QAudioSink>
#include <QtMultimedia/QMediaDevices>


class REVLENS_AUDIO_API AUDIO_Handler : public QObject {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    AUDIO_Handler(DS_ControllerBase* controller);

public slots:

    void init();

    void setTargetFrameRate(float frameRate);
    float getTargetFrameRate();

    void mute();
    float unmute();
    bool isMuted() { return _muted; }
    float volume();
    void setVolume(float volume);

    void resume();
    void stop(qlonglong frameNum);
    void updateToFrame(qlonglong frameNum);

    void clearAll();
    void toggleAudioScrubbing();

    void onScrubStart(qlonglong frameNum);
    void onScrubEnd(qlonglong frameNum);
    void onScrub(qlonglong frameNum);


private:

    DS_ControllerBase* _controller;

    QAudioFormat _audioFormat;
    QAudioSink* _audioSink;

    AUDIO_OutputDevice* _audioOutput;

    bool _scrubbingEnabled;
    bool _muted;
    float _volume;

};

#endif
