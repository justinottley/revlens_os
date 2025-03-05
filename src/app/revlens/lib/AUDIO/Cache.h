//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_AUDIO_CACHE_H
#define REVLENS_AUDIO_CACHE_H

#include "RlpRevlens/AUDIO/Global.h"

#include "RlpRevlens/DS/AudioBuffer.h"
#include "RlpRevlens/DS/ControllerBase.h"
#include "RlpRevlens/DS/Session.h"

#include "RlpRevlens/AUDIO/AudioTime.h"



class REVLENS_AUDIO_API AUDIO_Cache : public QObject {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

public:
    AUDIO_Cache(DS_SessionPtr session,
                QAudioFormat format,
                int samplesPerBuffer,
                int videoFramesPerCache);

    void update();

    static const int VIDEO_FRAME_BEHIND_PAD;

    // Playback relative state
    //

    qlonglong videoFramesAhead();
    AUDIO_TimePtr bufferPosition() { return _bufferPosition; }
    AUDIO_TimePtr endTime() { return _endTime; }

    // Indexes
    //

    QHash<qlonglong, DS_AudioBufferPtr>* index() { return &_audioIndex; }
    QList<qlonglong>* indexList() { return &_audioList; }
    QHash<qlonglong, qlonglong>* indexVF() { return &_audioVFIndex; }

    QHash<qlonglong, qlonglong>* videoFrameIndex() { return &_videoFrameIndex; }

    // Insertion
    //
    void incrementBufferPosition(int dataSize);
    void append(DS_AudioBufferPtr data);



public slots:

    void setVideoFrameRate(float videoFrameRate);
    void setPositionByVideoFrame(qlonglong videoFrame, bool doReset=true);

    void clear();

    // Accessors

    float videoFrameRate() { return _videoFrameRate;}
    qlonglong bufferIndexAtVideoFrame(qlonglong videoFrame);

    // Constants
    //
    QAudioFormat audioFormat() { return _kAudioFormat; }
    int byteCountPerBuffer() { return _kByteCountPerBuffer; }
    int samplesPerBuffer() { return _kSamplesPerBuffer; }
    qlonglong videoFramesPerCache() { return _kVideoFramesPerCache; }
    double byteCountPerVideoFrame() { return _byteCountPerVideoFrame; }

    uint64_t byteCountPerCache() const;
    uint64_t bytesAvailable();

    uint64_t maxSessionByteCount();

    void printInfo();

    QVariantMap toMap();



private:

    DS_SessionPtr _session;

    QAudioFormat _kAudioFormat;
    int _kSamplesPerBuffer;
    int _kVideoFramesPerCache;
    int _kByteCountPerBuffer;

    float _videoFrameRate;
    double _audioSamplesPerVideoFrame;
    double _byteCountPerVideoFrame;


    AUDIO_TimePtr _bufferPosition;
    AUDIO_TimePtr _endTime;

    // buffer index -> audio data
    QHash<qlonglong, DS_AudioBufferPtr> _audioIndex;

    // list of buffer indexes available (in order)
    QList<qlonglong> _audioList;

    // buffer index -> video frame
    QHash<qlonglong, qlonglong> _audioVFIndex;

    // tracks which video frames (worth of audio data)
    // are in the cache / is a videoFrame in the cache?
    //
    // QSet<qlonglong> _videoFrameIndex;

    // video frame -> start buffer index
    QHash<qlonglong, qlonglong> _videoFrameIndex;
};


#endif