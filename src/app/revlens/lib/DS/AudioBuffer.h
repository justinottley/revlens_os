//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_DS_AUDIOBUFFER_H
#define REVLENS_DS_AUDIOBUFFER_H

#include <memory>
#include <cstdlib>
#include <cstring>

#include <iostream>

#include "RlpRevlens/DS/Global.h"
#include "RlpRevlens/DS/Buffer.h"


#include <QtCore/QObject>
#include <QtCore/QMutex>
#include <QtCore/QVariantMap>


class DS_AudioBuffer;
typedef std::shared_ptr<DS_AudioBuffer> DS_AudioBufferPtr;

typedef QList<DS_AudioBufferPtr> DS_AudioBufferList;

class REVLENS_DS_API DS_AudioBuffer : public DS_Buffer {


public:

    DS_AudioBuffer(QObject* owner, QVariantMap bufferMetadata) :
        DS_Buffer(owner, bufferMetadata),
        _bytesPerSample(0),
        _channels(0),
        _samples(0),
        _dataSize(0)
    {
    }

    DS_AudioBuffer(QObject* owner) :
        DS_Buffer(owner, QVariantMap()),
        _bytesPerSample(0),
        _channels(0),
        _samples(0),
        _dataSize(0)
    {
    }

    DS_AudioBuffer(QObject* owner, int bitDepth, int channels, int samples) :
        DS_Buffer(owner, QVariantMap())
    {
        _bufferMetadata.insert("audio.output.bitdepth", bitDepth);
        _bufferMetadata.insert("audio.output.channels", channels);
        _bufferMetadata.insert("audio.samplesPerBuffer", samples);
        _bufferMetadata.insert("audio.read_result", READ_UNKNOWN);
    }


    ~DS_AudioBuffer()
    {
        free();
    }


    void free() {

        if (_data != NULL) {
            std::free(_data);

            _data = NULL;
            _dataBytes = 0;
            _dataSize = 0;
        }
    }

    virtual void reallocate();
    void reallocate(QVariantMap bufferMetadata);
    void reallocate(int bytesPerSample, int channels, int samples);

    void setDataSize(int dataSize);
    int getDataSize() { return _dataSize; }

    int samples() { return _samples; }
    int channels() { return _channels; }
    int bytesPerSample() { return _bytesPerSample; }

protected:

    int _bytesPerSample;
    int _channels;
    int _samples;

    // to allow readers to read less than the full buffer size
    // ffmpeg/wav i'm looking at you..
    //
    int _dataSize;


};

#endif

