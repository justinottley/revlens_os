//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include "RlpRevlens/DS/AudioBuffer.h"


void
DS_AudioBuffer::reallocate()
{
    if ((_dataBytes > 0) && (_data != NULL)) {
        std::free(_data);
        _data = NULL;
    }

    int bytesPerSample = _bufferMetadata.value("audio.output.bitdepth").value<int>();
    int channels = _bufferMetadata.value("audio.output.channels").value<int>();
    int samples = _bufferMetadata.value("audio.samplesPerBuffer").value<int>();

    if ((bytesPerSample == _bytesPerSample) &&
        (channels == _channels) &&
        (samples == _samples))
    {
        return;
    }

    _bytesPerSample = bytesPerSample;
    _channels = channels;
    _samples = samples;


    _dataBytes = (_bytesPerSample * _channels * _samples);
    _dataSize = _dataBytes;

    _data = std::malloc(_dataBytes);
    std::memset(_data, 0, _dataBytes);
}


void
DS_AudioBuffer::reallocate(QVariantMap bufferMetadata)
{
    _bufferMetadata = bufferMetadata;
    reallocate();
}


void
DS_AudioBuffer::reallocate(int bitDepth, int channels, int samples)
{
    _bufferMetadata.insert("audio.output.bitdepth", bitDepth);
    _bufferMetadata.insert("audio.output.channels", channels);
    _bufferMetadata.insert("audio.samplesPerBuffer", samples);

    reallocate();
}


void
DS_AudioBuffer::setDataSize(int dataSize)
{ 
    _dataSize = dataSize;
    _samples = dataSize / (_channels * _bytesPerSample);
}
