
#ifndef REVLENS_CNTRL_AUDIO_H
#define REVLENS_CNTRL_AUDIO_H

#include "RlpRevlens/CNTRL/Global.h"

#include "RlpRevlens/AUDIO/Handler.h"


class CNTRL_MainController;


class REVLENS_CNTRL_API CNTRL_Audio : public QObject {
    Q_OBJECT

signals:
    void setTargetFrameRateRequest(float frameRate);
    void muteRequest();
    void unmuteRequest();
    void setVolumeRequest(float volume);
    void resumeRequest();
    void stopRequest(qlonglong frameNum);
    void updateToFrameRequest(qlonglong frameNum);
    void clearAllRequest();

    void toggleAudioScrubbingRequest();

    void scrubStartRequest(qlonglong frameNum);
    void scrubEndRequest(qlonglong frameNum);
    void scrubRequest(qlonglong frame);

public:
    RLP_DEFINE_LOGGER

    CNTRL_Audio(CNTRL_MainController* controller);

    void setTargetFrameRate(float frameRate);
    float getTargetFrameRate();

    AUDIO_Handler* handler() { return _audioHandler; }

public slots:

    bool enabled() { return _enabled; }
    // void setEnabled(bool isEnabled);

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

    void onMediaAdded(DS_NodePtr node, QString uuidStr, qlonglong frameNum, qlonglong mediaFrameCount);
    void onNodeFlagStateChanged(QString nodeUuidStr, QString flagName, bool val);

private slots:
    void initAudioOutput();

private:
    CNTRL_MainController* _controller;

    QThread* _audioThread;
    AUDIO_Handler* _audioHandler;

    bool _enabled;
    
    bool _muted;
    float _volume;
    float _frameRate;
};

#endif
