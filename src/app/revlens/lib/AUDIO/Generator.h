//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_AUDIO_GENERATOR_H
#define REVLENS_AUDIO_GENERATOR_H

#include "RlpRevlens/AUDIO/Global.h"

#include "RlpCore/LOG/Logging.h"
#include "RlpRevlens/DS/ControllerBase.h"
#include "RlpRevlens/DS/AudioBuffer.h"

#include "RlpRevlens/AUDIO/Cache.h"
#include "RlpRevlens/AUDIO/WaveformCache.h"

static const int DATA_SAMPLE_RATE_HZ = 48000; // 44100;
static const int DATA_CHANNEL_COUNT = 2;
static const int DATA_SAMPLE_SIZE = 16;



class AUDIO_Manager;

class REVLENS_AUDIO_API AUDIO_Generator : public QIODevice {
    Q_OBJECT


signals:
    void bufferFillEvent(QList<qlonglong> frameList);


public:
    RLP_DEFINE_LOGGER

    AUDIO_Generator(DS_ControllerPtr controller);
    AUDIO_Generator(const QAudioFormat &format, DS_ControllerPtr controller);
    ~AUDIO_Generator();

    static QAudioFormat makeAudioFormat();

    void clear();
    void stop();


    void freeAvailableBuffers();

    qint64 readData(char *data, qint64 maxlen);

    void updateToFrame(qlonglong frameNum, bool recenter=false);

    qint64 writeData(const char *data, qint64 len);
    qint64 bytesAvailable() const;



public slots:

    AUDIO_Cache* cache() { return &_cache; }

    void setTargetFrameRate(float frameRate, bool fillBuffer=true);
    float getTargetFrameRate() { return _cache.videoFrameRate(); }

    virtual bool fillBuffer();
    virtual bool readAudioOnce(DS_NodePtr node = nullptr);

protected:

    DS_ControllerPtr _controller;

    QVariantMap _audioInfo;
    qlonglong _currAudioIndex;
    AUDIO_Cache _cache;

    DS_AudioBufferPtr _emptyBuffer;

    QMutex _lock;
};


#endif
