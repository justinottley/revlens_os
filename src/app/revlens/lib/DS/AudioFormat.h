
#ifndef REVLENS_DS_AUDIO_FORMAT_H
#define REVLENS_DS_AUDIO_FORMAT_H

#include "RlpRevlens/DS/Global.h"

#ifdef __MACH__
static const int AUDIO_SINK_BUFFER_SIZE = 32768; // 262144; // 131072; // 65536; // 32768; // 16384; // 8192; // 4096; // 2048; // 1024; // 512; // 204800;
#else
static const int AUDIO_SINK_BUFFER_SIZE = 262144;
#endif

static const int AUDIO_DATA_SAMPLE_RATE_HZ = 48000; // 44100;
static const int AUDIO_DATA_CHANNEL_COUNT = 2;
static const int AUDIO_DATA_BYTES_PER_SAMPLE = 2;


class DS_AudioFormat {

public:
    DS_AudioFormat(float frameRate);

    int samplesPerFrame();
    int bytesPerFrame();

    float frameRate() { return _frameRate; }
    void setFrameRate(float frameRate) { _frameRate = frameRate; }

    qint64 bytePositionForFrame(qlonglong frameNum);
    qint32 samplesForBytes(qint64 byteCount);

private:
    float _frameRate;

};

#endif