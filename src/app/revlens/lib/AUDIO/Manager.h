//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_AUDIO_MANAGER_H
#define REVLENS_AUDIO_MANAGER_H

#include "RlpRevlens/AUDIO/Global.h"

#include "RlpCore/LOG/Logging.h"
#include "RlpRevlens/DS/Node.h"
#include "RlpRevlens/DS/Session.h"
#include "RlpRevlens/DS/ControllerBase.h"

class AUDIO_Mixer;

// static const int BUFFER_SIZE = 16384; // 32768; // (32768 / 2); // 65536;

static const int BUFFER_SIZE = 204800; // 32768; // (32768 / 2); // 65536;

class REVLENS_AUDIO_API AUDIO_Manager : public QObject {
    Q_OBJECT
    
public:
    RLP_DEFINE_LOGGER

    AUDIO_Manager(DS_ControllerPtr controller);
    ~AUDIO_Manager();

    DS_ControllerPtr getController() { return _controller; }

    void setVolume(float volume);

    void scheduleGenerateWaveform(DS_Node* node);

public slots:

    void resume();
    void suspend();

    void mute();
    void unmute();

    void initializeAudio();

    void updateToFrame(qlonglong frame, bool recenter=false);
    // void stop(qlonglong frame);

    void fillBuffer();

    AUDIO_Mixer* mixer();

    void setTargetFrameRate(float frameRate);
    float getTargetFrameRate();

    // void onPlaylistUpdated(QList<DS_NodePtr>*, qlonglong);

    bool enabled() { return _enabled; }
    void setEnabled(bool isEnabled);

    void onSessionChanged(DS_SessionPtr session);
    void onMediaAdded(DS_NodePtr node, QString uuidStr, qlonglong frameNum, qlonglong mediaFrameCount);
    void onNodeDataReady(QVariantMap data);

signals:

    void mgrAudioReady();


private:


    void createAudioOutput();

private slots:

    void notified();
    void handleStateChanged(QAudio::State state);

private:

    DS_ControllerPtr _controller;

    QAudioSink* _audioOutput;

    QAudioFormat _format;

    AUDIO_Mixer* _mixer;

    // QIODevice* _output;

    bool _enabled;
    float _volume;
    
    // QThread _bufferThread;
};

#endif
