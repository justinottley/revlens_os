//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include "RlpCore/LOG/Logging.h"
#include "RlpCore/UTIL/AppGlobals.h"

#include "RlpRevlens/AUDIO/Cache.h"

const int AUDIO_Cache::VIDEO_FRAME_BEHIND_PAD = 10;

RLP_SETUP_LOGGER(revlens, AUDIO, Cache)

AUDIO_Cache::AUDIO_Cache(DS_SessionPtr session,
                         QAudioFormat format,
                         int samplesPerBuffer,
                         int videoFramesPerCache) :
    _session(session),
    _videoFrameRate(0),
    _kAudioFormat(format),
    _kSamplesPerBuffer(samplesPerBuffer),
    _kVideoFramesPerCache(videoFramesPerCache),
    _audioSamplesPerVideoFrame(0),
    _byteCountPerVideoFrame(0),
    _bufferPosition(new AUDIO_Time(0, format.sampleRate(), format.bytesPerFrame())),
    _endTime(new AUDIO_Time(0, format.sampleRate(), format.bytesPerFrame()))
{
    _kByteCountPerBuffer = _kAudioFormat.bytesPerFrame() * _kSamplesPerBuffer;

    // Put audio setting constants into appglobals
    //
    QVariantMap* ag = UTIL_AppGlobals::instance()->globalsPtr();
    ag->insert(toMap());

    update();
}


void
AUDIO_Cache::update()
{
    RLP_LOG_DEBUG("");

    if (_videoFrameRate == 0)
    {
        RLP_LOG_DEBUG("frame rate not set, skipping update");
        return;
    }

    _audioSamplesPerVideoFrame = (double)_kAudioFormat.sampleRate() / (double)_videoFrameRate;
    _byteCountPerVideoFrame = _audioSamplesPerVideoFrame * _kAudioFormat.bytesPerFrame();

    // printInfo();
}


void
AUDIO_Cache::setVideoFrameRate(float videoFrameRate)
{
    RLP_LOG_DEBUG(videoFrameRate);

    _videoFrameRate = videoFrameRate;
    _endTime->setVideoFrameRate(videoFrameRate);
    _bufferPosition->setVideoFrameRate(videoFrameRate);
    update();
}


uint64_t
AUDIO_Cache::byteCountPerCache() const
{
    return _byteCountPerVideoFrame * _kVideoFramesPerCache;
}


uint64_t
AUDIO_Cache::bytesAvailable()
{
    uint64_t currSize = _videoFrameIndex.size() * _byteCountPerVideoFrame;
    uint64_t cacheSize = byteCountPerCache();

    if (currSize < cacheSize) {
        return cacheSize - currSize;
    }

    return 0;
}


qlonglong
AUDIO_Cache::videoFramesAhead()
{
    qlonglong currFrame = bufferPosition()->videoFrame();
    qlonglong endFrame = endTime()->videoFrame();
    qlonglong playlistMaxFrame = _session->frameCount();

    qlonglong result = -1;
    if (currFrame <= endFrame) {
        result = endFrame - currFrame;
    } else {
        result = (playlistMaxFrame - currFrame) + endFrame;
    }

    return result;
}


uint64_t
AUDIO_Cache::maxSessionByteCount()
{
    return _session->frameCount() * _byteCountPerVideoFrame;
}


void
AUDIO_Cache::clear()
{
    // RLP_LOG_DEBUG("")

    _audioIndex.clear();
    _audioList.clear();
    _audioVFIndex.clear();

    _videoFrameIndex.clear();

    _endTime->setBufferIndex(0);
    _endTime->setBytes(0);

    _bufferPosition->setBufferIndex(0);
    _bufferPosition->setBytes(0);
}


void
AUDIO_Cache::append(DS_AudioBufferPtr data)
{
    // RLP_LOG_DEBUG("appending at " << _endTime->videoFrame() << " index: " << _endTime->bufferIndex() << " " << _endTime->bytes() << " " << maxSessionByteCount());

    _audioIndex.insert(_endTime->bufferIndex(), data);
    _audioList.push_back(_endTime->bufferIndex());
    _audioVFIndex[_endTime->bufferIndex()] = _endTime->videoFrame();

    if (!_videoFrameIndex.contains(_endTime->videoFrame())) {
        // _videoFrameIndex.insert(_endTime->videoFrame());
        // RLP_LOG_DEBUG("register " << _endTime->videoFrame() << " with index " << _endTime->bufferIndex());
        _videoFrameIndex[_endTime->videoFrame()] = _endTime->bufferIndex();
    }


    int bufferSizeDiff = maxSessionByteCount() - (_endTime->bytes() + data->getDataSize());

    // RLP_LOG_DEBUG("AUDIO_Cache::append(): buffer size diff: " << bufferSizeDiff << " byte count per buffer: " << byteCountPerBuffer());



    // setBytes() will eventually set videoFrame
    //
    _endTime->setBufferIndex(_endTime->bufferIndex() + 1);
    _endTime->setBytes(_endTime->bytes() + data->getDataSize());

}


void
AUDIO_Cache::incrementBufferPosition(int dataSize)
{
    // int bufferSizeDiff = maxSessionByteCount() - (_bufferPosition->bytes() + dataSize);

    _bufferPosition->setBufferIndex(_bufferPosition->bufferIndex() + 1);
    _bufferPosition->setBytes(_bufferPosition->bytes() + dataSize);

    /*
    RLP_LOG_DEBUG("AUDIO_Cache::incrementBufferPosition() : "
              << _bufferPosition->bufferIndex()
              << " byte count: " << _bufferPosition->bytes()
              << " max byte count: " << maxSessionByteCount()
             );

    */
}



void
AUDIO_Cache::setPositionByVideoFrame(qlonglong videoFrame, bool doReset)
{
    // RLP_LOG_DEBUG(videoFrame << "VFI size: " << videoFrameIndex()->size() << "MAX: " << videoFramesPerCache())

    // bool doReset = (videoFrameIndex()->size() >= videoFramesPerCache());
    if (doReset) {
        // RLP_LOG_DEBUG("CACHE RESET!!!");
        clear();
    }


    // uint64_t bufferBytesAtVideoFrame = videoFrame * _byteCountPerVideoFrame;
    //
    // NOTE: approxmiate..?
    // qlonglong bufIndexAtVideoFrame = bufferBytesAtVideoFrame / _kByteCountPerBuffer;

    qlonglong bufIndexAtVideoFrame = bufferIndexAtVideoFrame(videoFrame);
    // RLP_LOG_DEBUG(videoFrame << " using buffer index: " << bufIndexAtVideoFrame);

    _bufferPosition->setVideoFrame(videoFrame);
    _bufferPosition->setBufferIndex(bufIndexAtVideoFrame);

    if (doReset)
    {

        // RLP_LOG_DEBUG("CACHE RESET!!");
        _endTime->setVideoFrame(videoFrame);
        _endTime->setBufferIndex(bufIndexAtVideoFrame);
    }

}


qlonglong
AUDIO_Cache::bufferIndexAtVideoFrame(qlonglong videoFrame)
{
    uint64_t bufferBytesAtVideoFrame = videoFrame * _byteCountPerVideoFrame;

    // NOTE: approxmiate..?
    qlonglong bufIndexAtVideoFrame = bufferBytesAtVideoFrame / _kByteCountPerBuffer;

    return bufIndexAtVideoFrame;
}


void
AUDIO_Cache::printInfo()
{
    RLP_LOG_DEBUG("");
    RLP_LOG_DEBUG("    Video Frame Rate: " << _videoFrameRate);
    RLP_LOG_DEBUG("    Sample Rate: " << _kAudioFormat.sampleRate());
    RLP_LOG_DEBUG("");
    RLP_LOG_DEBUG("    Samples Per Video Frame: " << _audioSamplesPerVideoFrame);
    RLP_LOG_DEBUG("    Byte Count per Video Frame: " << _byteCountPerVideoFrame);
    RLP_LOG_DEBUG("    ");
    RLP_LOG_DEBUG("    Samples per buffer: " << _kSamplesPerBuffer);
    RLP_LOG_DEBUG("    Byte Count per buffer: " << _kByteCountPerBuffer);
    RLP_LOG_DEBUG("");
    RLP_LOG_DEBUG("    Max Playlist Frame: " << _session->frameCount());
    RLP_LOG_DEBUG("");
    RLP_LOG_DEBUG("    Current Buffer Index: " << _bufferPosition->bufferIndex());
    RLP_LOG_DEBUG("    Current End Position secs: " << _endTime->secs());
    RLP_LOG_DEBUG("    Current End Position videoFrame: " << _endTime->videoFrame());
    RLP_LOG_DEBUG("    Current Size: " << _audioIndex.size());
}


QVariantMap
AUDIO_Cache::toMap()
{
    QVariantMap audioInfo;

    audioInfo.insert("audio.output.bitdepth", _kAudioFormat.bytesPerSample());
    audioInfo.insert("audio.output.channels", _kAudioFormat.channelCount());
    audioInfo.insert("audio.output.sampleRate", _kAudioFormat.sampleRate());

    audioInfo.insert("audio.samplesPerBuffer", _kSamplesPerBuffer);
    audioInfo.insert("audio.byteCountPerBuffer", _kByteCountPerBuffer);
    audioInfo.insert("audio.samplesPerVideoFrame", _audioSamplesPerVideoFrame);
    audioInfo.insert("audio.byteCountPerVideoFrame", _byteCountPerVideoFrame);

    audioInfo.insert("audio.buffer.index", _bufferPosition->bufferIndex());

    return audioInfo;
}


