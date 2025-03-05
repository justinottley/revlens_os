//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_AUDIO_TIME_H
#define REVLENS_AUDIO_TIME_H

#include "RlpRevlens/DS/Time.h"
#include "RlpRevlens/AUDIO/Global.h"

class AUDIO_Time;
typedef std::shared_ptr<AUDIO_Time> AUDIO_TimePtr;

class REVLENS_AUDIO_API AUDIO_Time : public DS_Time {

public:

    AUDIO_Time(float videoFrameRate, int samplesPerSec, int bytesPerSample):
        DS_Time(videoFrameRate),
        _samplesPerSec(samplesPerSec),
        _bytesPerSample(bytesPerSample),
        _bufferIndex(0)
    {
    }


    uint64_t samples() { return uint(round(secs() * _samplesPerSec)); }
    uint64_t bytes() { return samples() * _bytesPerSample; }
    int bufferIndex() { return _bufferIndex; }

    void setSamples(int samples) { 
        float secs = (float)samples / (float)_samplesPerSec;
        setSecs(secs);

        // std::cout << "setSamples(): " << secs << " " << samples << " " << _secs << std::endl;
    }

    void setBytes(uint64_t bytes) {

        // Ideally this should be an int...
        float samples = (float)bytes / (float)_bytesPerSample;
        setSamples(samples);
    }

    void setBufferIndex(int bufferIndex) { 
        _bufferIndex = bufferIndex; 
    }


    AUDIO_TimePtr copy()
    {
        AUDIO_TimePtr result(new AUDIO_Time(_videoFrameRate, _samplesPerSec, _bytesPerSample));
        result->setVideoFrame(_videoFrame);
        result->setBufferIndex(_bufferIndex);

        return result;
    }

protected:

    int _samplesPerSec;
    int _bytesPerSample;

    int _bufferIndex;

};

#endif
