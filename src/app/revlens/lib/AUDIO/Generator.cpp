//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include "RlpRevlens/AUDIO/Generator.h"
#include "RlpRevlens/DS/PlaybackMode.h"


RLP_SETUP_LOGGER(revlens, AUDIO, Generator)

QAudioFormat
AUDIO_Generator::makeAudioFormat()
{
    QAudioFormat format;
    format.setSampleRate(DATA_SAMPLE_RATE_HZ);
    // format.setChannelCount(DATA_CHANNEL_COUNT);
    // format.setSampleSize(DATA_SAMPLE_SIZE);
    // format.setCodec("audio/pcm");
    format.setSampleFormat(QAudioFormat::Int16);
    format.setChannelConfig(QAudioFormat::ChannelConfigStereo);
    // format.setByteOrder(QAudioFormat::LittleEndian);
    // format.setSampleType(QAudioFormat::SignedInt);


    const QAudioDevice ddevice = QMediaDevices::defaultAudioOutput();

    if (!ddevice.isFormatSupported(format))
    {

        RLP_LOG_WARN("Default audio format not supported - trying to use preferred");
        // format = info.nearestFormat(format);

        QList<QAudioDevice> availableDevices = QMediaDevices::audioOutputs();

        RLP_LOG_DEBUG("number of available output devices: " << availableDevices.size());

        format = ddevice.preferredFormat();

        // RLP_LOG_WARN("  codec: " << format.codec());
        // RLP_LOG_WARN("  byteOrder: " << (int)format.byteOrder());
        RLP_LOG_WARN("  channelCount: " << format.channelCount());
        RLP_LOG_WARN("  sampleRate: " << format.sampleRate());
        RLP_LOG_WARN("  sampleFormat: " << format.sampleFormat());
        //RLP_LOG_WARN("  sampleType: " << format.sampleType());

    } else
    {
        // RLP_LOG_DEBUG("format good");
    }

    return format;
}


AUDIO_Generator::AUDIO_Generator(const QAudioFormat &format,
                                 DS_ControllerPtr controller):
    QIODevice(),
    _controller(controller),
    _currAudioIndex(0),
    _cache(
        controller->session(),
        format,
        256 /* samples per buffer */,
        90 /* video frames per cache */
    )
{
    // RLP_LOG_DEBUG("");

    _audioInfo.insert("audio.output.bitdepth", _cache.audioFormat().bytesPerSample());
    _audioInfo.insert("audio.output.channels", _cache.audioFormat().channelCount());
    _audioInfo.insert("audio.samplesPerBuffer", _cache.samplesPerBuffer());

    _emptyBuffer = DS_AudioBufferPtr(new DS_AudioBuffer(this, _audioInfo));
    _emptyBuffer->reallocate();

    std::memset(_emptyBuffer->data(), 0, _emptyBuffer->getDataBytes());
}


AUDIO_Generator::~AUDIO_Generator()
{
}


void
AUDIO_Generator::clear()
{
    QMutexLocker l(&_lock);

    _cache.clear();
    _cache.update();
}


void
AUDIO_Generator::setTargetFrameRate(float frameRate, bool doFillBuffer)
{
    RLP_LOG_DEBUG(frameRate << " " << _cache.endTime()->secs());

    _cache.setVideoFrameRate(frameRate);
    if ((_cache.indexList()->size() == 0) && (doFillBuffer))
    {
        fillBuffer();
    }
}


void
AUDIO_Generator::freeAvailableBuffers()
{
    qlonglong currFrame = _cache.bufferPosition()->videoFrame();
    qlonglong endFrame = _cache.endTime()->videoFrame();
    qlonglong framesAhead = _cache.videoFramesAhead();

    qlonglong framesAheadMax = (_cache.videoFramesPerCache() / 2) - 20;

    /*
    RLP_LOG_DEBUG("AUDIO_Generator::freeAvailableBuffers(): "
              << " curr frame: " << currFrame
              << " end frame: " << endFrame
              << " frames ahead: " << framesAhead
             );
    */


    if ((_cache.maxSessionByteCount() < _cache.byteCountPerCache()) || (framesAhead > framesAheadMax))
    {

        /*
        RLP_LOG_DEBUG("AUDIO_Generator::freeAvailableBuffers(): SKIPPING CLEAN");
        RLP_LOG_DEBUG(" maxSessionByteCount: " << _cache.maxSessionByteCount());
        RLP_LOG_DEBUG(" byteCountPerCache: " << _cache.byteCountPerCache());
        RLP_LOG_DEBUG(" framesAhead: " << framesAhead);
        RLP_LOG_DEBUG(" framesAheadMax: " << framesAheadMax);
        */

        return;
    }


    QList<qlonglong>::iterator it;

    /*
    RLP_LOG_DEBUG("AUDIO_Generator::freeAvailableBuffers(): ";
    for (it = _cache.indexList()->begin(); it != _cache.indexList()->end(); ++it) {
        RLP_LOG_DEBUG((*it) << " ";
    }
    RLP_LOG_DEBUG(std::endl;
    */


    int posCount = 0;
    QSet<qlonglong> removeFrameSet;

    qlonglong videoFrameRemoveCount = (_cache.videoFramesPerCache() / 2) - AUDIO_Cache::VIDEO_FRAME_BEHIND_PAD;

    // RLP_LOG_DEBUG("VIDEO FRAME REMOVE COUNT: " << videoFrameRemoveCount);

    for (it = _cache.indexList()->begin(); it != _cache.indexList()->end(); ++it)
    {

        if (removeFrameSet.size() >= videoFrameRemoveCount)
        {
            // RLP_LOG_DEBUG("AUDIO_Generator::freeAvailableBuffers(): REMOVED " << videoFrameRemoveCount << " FRAMES WORTH OF AUDIO, BREAKING");
            break;
        }


        if (_cache.indexVF()->contains(*it))
        {

            qlonglong videoFrame = _cache.indexVF()->value(*it);
            removeFrameSet.insert(videoFrame);

            // RLP_LOG_DEBUG(" REMOVED AT : " << removeFrameSet.size();

            if (_cache.videoFrameIndex()->contains(videoFrame))
            {
                _cache.videoFrameIndex()->remove(videoFrame);

                // RLP_LOG_DEBUG("AUDIO_Generator::freeAvailableBuffers(): REMOVING  " << videoFrame);

            } else
            {
                // RLP_LOG_DEBUG("VIDEO FRAME INDEX DOES NOT HAVE FRAME: " << videoFrame);
            }

            int removeCount = _cache.index()->remove((*it));

            // RLP_LOG_DEBUG("remove count: " << removeCount << " FREE " << (*it) << " " << videoFrame);

            posCount++;


        } else
        {

            RLP_LOG_DEBUG("AUDIO VF INDEX DOES NOT HAVE BUFFER " << (*it));
        }

    }

    // RLP_LOG_DEBUG(std::endl;

    for (int x=0; x != posCount; ++x)
    {
        _cache.indexList()->removeFirst();
    }


    /*
    RLP_LOG_DEBUG("AUDIO_Generator::freeAvailableBuffers(): freed "
              << posCount << " buffers - end audio read cache index size: "
              << _cache.index()->size());
    */
}


bool
AUDIO_Generator::readAudioOnce(DS_NodePtr node)
{
    // RLP_LOG_DEBUG(_cache.endTime()->videoFrame())

    qlonglong videoFrameOffset = 0;
    if (node == nullptr)
    {
        node = _controller->session()->getNodeByFrame(_cache.endTime()->videoFrame());
    }

    if (node == nullptr)
    {
        // RLP_LOG_DEBUG("NO NODE AT FRAME " << _cache.endTime()->videoFrame() << " aborting fill");
        return false;
    }

    if (!(node->hasProperty("audio.format.reader")))
    {
        // RLP_LOG_DEBUG("NO AUDIO READER: aborting");
        return false;
    }

    videoFrameOffset = _controller->session()->getNodeFrameIndex(node);


    // NOTE: UNALLOCATED
    //
    DS_AudioBufferPtr ab(new DS_AudioBuffer(this));

    AUDIO_TimePtr time = _cache.endTime()->copy();
    time->setVideoFrameOffset(videoFrameOffset);

    /*
    RLP_LOG_DEBUG("AUDIO_Generator::readAudioOnce(): reading at frame "
                << time->videoFrame() << " "
                << time->mediaVideoFrame() << " "
                << "bufferIndex: " << _cache.endTime()->bufferIndex() << " "
                << "videoFrameOffset: " << _cache.endTime()->videoFrameOffset() << " "
                << "videoFrame: " << _cache.endTime()->videoFrame()
                << " from node " << node->getProperty<QString>("media.name").toStdString().c_str());
    */

    node->readAudio(time, ab);


    //
    // node->scheduleReadTask(f, ab, DS_Node::DATA_AUDIO);


    // RLP_LOG_DEBUG("AUDIO_Generator::readAudioOnce(): GOT DATA SIZE:" << ab->getDataSize());

    int nb_frames = _cache.audioFormat().framesForBytes(ab->getDataSize()); //  / _format.bytesPerFrame();
    float nb_buffers_fl = (float)nb_frames / (float)_cache.samplesPerBuffer();
    int nb_buffers = int(nb_buffers_fl);

    // Add remainder into a last buffer
    if ((nb_buffers_fl - nb_buffers) > 0)
    {
        nb_buffers++;
    }

    /*
    RLP_LOG_DEBUG("Num Frames From Size: "
                << _cache.audioFormat().framesForBytes(ab->getDataSize())
                << " num buffers from frames: " << nb_buffers_fl << " "  << nb_buffers
                << "READ " << ab->getDataBytes() << " bytes "
                << nb_frames << " frames "
                << nb_buffers << " buffers");
    */


    int nodeReadBytesRemaining = ab->getDataSize();
    int nodeReadBufSize = 0;


    for (int x=0; x != nb_buffers; ++x)
    {


        DS_AudioBufferPtr buf(new DS_AudioBuffer(this, _audioInfo));
        buf->reallocate();

        int bufSize = buf->getDataBytes();
        if (bufSize > nodeReadBytesRemaining)
        {
            bufSize = nodeReadBytesRemaining;
            buf->setDataSize(nodeReadBytesRemaining);
        }

        // RLP_LOG_DEBUG("BUFFER SPLIT " << x << " with " << buf->getDataBytes() << " bytes");


        memcpy(
            buf->data(),
            (char *)ab->data() + nodeReadBufSize,
            bufSize);

        _cache.append(buf);


        nodeReadBytesRemaining -= buf->getDataBytes();
        nodeReadBufSize += bufSize;

        int16_t* bufData = (int16_t*)buf->data();
    }

    ab->free();

    int audioReadResult = ab->getBufferMetadata()->value("audio.read_result").toInt();
    if (audioReadResult == (int)DS_Buffer::READ_EOF_OK)
    {

        // RLP_LOG_DEBUG("AUDIO_Generator::readAudioOnce(): READ END OF FILE!!!");

        // try advancing to the next frame?
        _cache.endTime()->setVideoFrame(_cache.endTime()->videoFrame() + 1);

        return false;
    }
    else if (audioReadResult == (int)DS_Buffer::READ_NONE)
    {
        return false;
    }

    return true;


    /*
    RLP_LOG_DEBUG("END TIME: "
                << _cache.endTime()->videoFrame()
                << " END BUFFER INDEX: " << _cache.endTime()->bufferIndex()
                << " VFI SIZE: " << _cache.videoFrameIndex()->size()
                << " MEDIA END TIME: " << _cache.endTime()->mediaVideoFrame()
                << " playlist max frames: " << _controller->getPlaylist()->getTotalNumFrames()
                << " curr index: " << _currAudioIndex
                << " next index: " << nextNodeIndex
                << " bytes available: " << _cache.bytesAvailable()
                << " totalBytes read: " << totalBytes
               );
    */
    /*
    RLP_LOG_DEBUG("READ BYTE TOTAL: " << totalBytes
                << " BUFFERS READ: " << buffersRead
                << " audio index size: " << _cache.index()->size()
                << " audio curr idx: " << _currAudioIndex
                << " End position bytes: " << _cache.endTime()->bytes()
               );

    */

}


bool
AUDIO_Generator::fillBuffer()
{
    QMutexLocker l(&_lock);

    // RLP_LOG_DEBUG("cache endTime videoFrame(): " << _cache.endTime()->videoFrame());

    freeAvailableBuffers();

    /*
    RLP_LOG_DEBUG("FILLING AT " << _cache.endTime()->bufferIndex()
              << " VFIsize: " << _cache.videoFrameIndex()->size()
              << " frame: " << _cache.endTime()->videoFrame()
              << " Bytes available in cache: " << _cache.bytesAvailable()
              << " Max playlist byte count: " << _cache.maxSessionByteCount()
              << " Cache Byte Count: " << _cache.byteCountPerCache()
             );
    */


    if (_cache.maxSessionByteCount() == 0) {
        // RLP_LOG_DEBUG("MAX SESSION BYTE COUNT IS ZERO, ABORTING FILL");
        return false;
    }



    while (true) {

        /*
        RLP_LOG_DEBUG("VFI size: " << _cache.videoFrameIndex()->size()
                  //<< " MAX: " << _cache.videoFramesPerCache()
                  // << " PLTOTAL: " << _controller->getPlaylist()->getTotalNumFrames()
                  << " ENDTIME: " << _cache.endTime()->videoFrame()
                  << " OUTFRAME: " << _controller->playbackMode()->getOutFrame()
                  << " SESSION FRAMECOUNT: " << _controller->session()->frameCount()
                 );
        */


        if ((_cache.videoFrameIndex()->size() > _cache.videoFramesPerCache())) {
            // RLP_LOG_DEBUG("buffer full, exit");
            break;
        }


        if (_cache.endTime()->videoFrame() > _controller->playbackMode()->getOutFrame()) {

            if (_cache.videoFrameIndex()->size() >= _controller->playbackMode()->getFrameCount()) {
                 // RLP_LOG_DEBUG("no more frames to read, breaking");
                 break;
            }

            if (_controller->playbackMode()->loopMode() == DS_PlaybackMode::LOOP) {

                    // RLP_LOG_DEBUG("END TIME WRAPAROUND, resetting end time");

                    _cache.endTime()->setVideoFrameOffset(0);
                    _cache.endTime()->setBufferIndex(0);
                    _cache.endTime()->setBytes(0);
                    _cache.endTime()->setVideoFrame(_controller->playbackMode()->getInFrame());

            } else {

                // RLP_LOG_DEBUG("PLAYBACKMODE LOOPMODE NOT LOOPING, END OF AUDIO FILL");

                break;

            }
        }


        bool readResult = readAudioOnce();
        if (!readResult) {
            break;
        }
    }


    // RLP_LOG_DEBUG("BREAKING OUT OF FILL BUFFER");


    // Make copy
    QList<qlonglong> audioIndex = _cache.videoFrameIndex()->keys();



    //RLP_LOG_DEBUG("______");

    /*
    RLP_LOG_DEBUG("AUDIO_Generator::fillBuffer(): ")

    QList<qlonglong>::iterator it;
    for (it=_cache.indexList()->begin(); it != _cache.indexList()->end(); ++it) {
        std::cout << *it << " ";
    }

    std::cout << std::endl;

    */
    // RLP_LOG_DEBUG("VIDEO FRAME INDEX SIZE: " << audioIndex.size());

    emit bufferFillEvent(audioIndex);


    return false;
}



void
AUDIO_Generator::stop()
{
    close();
}


void
AUDIO_Generator::updateToFrame(qlonglong videoFrameNum, bool recenter)
{
    // RLP_LOG_DEBUG("AUDIO_Generator::updateToFrame() : " << videoFrameNum);

    if (videoFrameNum < 0) {
        return;
    }

    _lock.lock();
    _cache.setPositionByVideoFrame(videoFrameNum);
    _lock.unlock();
    // _cache.printInfo();

    if (recenter)
    {
        fillBuffer();
    }
}


qint64
AUDIO_Generator::bytesAvailable() const
{

    qint64 result = _cache.byteCountPerCache() + QIODevice::bytesAvailable();
    // RLP_LOG_DEBUG(result);

    return result;
}


qint64
AUDIO_Generator::readData(char* data, qint64 len)
{
    // return 0;

    int num_buffers_to_read = len / _cache.byteCountPerBuffer();

    /*
    RLP_LOG_DEBUG("AUDIO_Generator::readData() : " << len
              << " byteCountPerBuffer: " << _cache.byteCountPerBuffer()
              << " audioIndexPos: " << _cache.bufferPosition()->bufferIndex()
              << " num buffers to read: " << num_buffers_to_read
             );
    */


    int remainder = len;
    int bytesToRead = 0;

    int bytesRead = 0;

    // int maxBytes = 28000 * 2 * 2;

    _lock.lock();

    for (int nb = 0; nb != num_buffers_to_read; ++nb)
    {

        AUDIO_TimePtr bp = _cache.bufferPosition();

        /*
        RLP_LOG_DEBUG("AUDIO_Generator::readData(): "
                  << " BPframe: " << bp->videoFrame()
                  << " BPindex: " << bp->bufferIndex()
                  << " outframe: " << _controller->playbackMode()->getOutFrame()
                 );
        */



        if (bp->videoFrame() > _controller->playbackMode()->getOutFrame())
        {

            if (_cache.index()->contains(bp->bufferIndex() + 1))
            {

                RLP_LOG_WARN("AT END BUT INDEX HAS MORE BUFFERS:"
                          << " BPframe: " << bp->videoFrame()
                          << " BPindex: " << bp->bufferIndex()
                         );

            } else
            {

                // RLP_LOG_DEBUG("PLAYHEAD END TIME WRAPAROUND!");

                _cache.bufferPosition()->setBufferIndex(0);
                _cache.bufferPosition()->setBytes(0);
                _cache.bufferPosition()->setVideoFrame(_controller->playbackMode()->getInFrame());

                _lock.unlock();

                fillBuffer();

                _lock.lock();

            }
        }



        int bytesToRead = 0;
        if (_cache.index()->contains(bp->bufferIndex()))
        {

            // int bytesToRead = 1024;

            // RLP_LOG_DEBUG("AUDIO_Generator::readData(): reading at " << bp->bufferIndex());

            DS_AudioBufferPtr acBuf = _cache.index()->value(bp->bufferIndex());

            bytesToRead = acBuf->getDataSize();


            memcpy(
                data + bytesRead,
                (char *)acBuf->data(),
                acBuf->getDataSize());


            bytesRead += bytesToRead;



        } else
        {

            // RLP_LOG_ERROR("MASTER AUDIO INDEX DOES NOT HAVE THIS BUFFER: "
            //          << bp->bufferIndex() << " " << bp->videoFrame() << " " << _emptyBuffer->getDataSize());

            memcpy(data + bytesRead,
                   (char *)_emptyBuffer->data(),
                   _emptyBuffer->getDataSize());

            bytesRead += _emptyBuffer->getDataSize();
            bytesToRead = _emptyBuffer->getDataSize();
        }


        _cache.incrementBufferPosition(bytesToRead);

        /*
        RLP_LOG_DEBUG("TOTAL BYTES READ: "
                  << _cache.bufferByteCount()
                  << " TOTAL PLAYLIST BYTES: "
                  << _controller->getPlaylist()->getTotalNumAudioFrames() * 4
                 );

        */

    }

    _lock.unlock();

    /*
    RLP_LOG_DEBUG("BUFFER BYTES: " << _cache.bufferByteCount()
              << " GLOBAL BYTES:" << _globalBytesRead
              << " BUFFER INDEX: " << _cache.bufferPosition()->bufferIndex()
              << " AUDIOINDEX COUNT: " << _cache.index()->count()
              << " END POSITION: " << _cache.endPosition()
              << " BUFFERS PER CACHE: " << _cache.buffersPerCache()
             );


    */


    fillBuffer();


    // NOTE: this may not be the exact amount of bytes requested in len, might
    // be less
    return bytesRead;

}


qint64
AUDIO_Generator::writeData(const char *data, qint64 len)
{
    Q_UNUSED(data);
    Q_UNUSED(len);

    return 0;
}

