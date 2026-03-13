
#include "RlpRevlens/DS/AudioFormat.h"


DS_AudioFormat::DS_AudioFormat(float frameRate):
    _frameRate(frameRate)
{
}


int
DS_AudioFormat::samplesPerFrame()
{
    return int(rint(AUDIO_DATA_SAMPLE_RATE_HZ / _frameRate));
}


int
DS_AudioFormat::bytesPerFrame()
{
    return samplesPerFrame() * AUDIO_DATA_CHANNEL_COUNT * AUDIO_DATA_BYTES_PER_SAMPLE;
}


qint64
DS_AudioFormat::bytePositionForFrame(qlonglong frameNum)
{
    return frameNum * bytesPerFrame();
}


qint32
DS_AudioFormat::samplesForBytes(qint64 byteCount)
{
    int sampleByteCount = AUDIO_DATA_CHANNEL_COUNT * AUDIO_DATA_BYTES_PER_SAMPLE;
    return int(rint((float)byteCount / (float)sampleByteCount));

}