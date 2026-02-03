
#include "RlpRevlens/AUDIO/Mixer.h"

RLP_SETUP_LOGGER(revlens, AUDIO, Mixer)
RLP_SETUP_LOGGER(revlens, AUDIO, FillBufferTask)

AUDIO_FillBufferTask::AUDIO_FillBufferTask(AUDIO_Mixer* mixer):
    _mixer(mixer)
{
}


void
AUDIO_FillBufferTask::run()
{
    _mixer->_fillBufferImpl();
}


AUDIO_Mixer::AUDIO_Mixer(const QAudioFormat &format, DS_ControllerPtr controller):
    AUDIO_Generator(format, controller)
{
    _tp.setMaxThreadCount(1);
}


void
AUDIO_Mixer::initAudioOutput()
{
    RLP_LOG_DEBUG("")

    open(QIODevice::ReadOnly);
}


bool
AUDIO_Mixer::fillBuffer()
{
    _tp.start(new AUDIO_FillBufferTask(this));

    return true;
}


bool
AUDIO_Mixer::_fillBufferImpl()
{
    return AUDIO_Generator::fillBuffer();
}


bool
AUDIO_Mixer::readAudioOnce(DS_NodePtr node)
{
    // return AUDIO_Generator::readAudioOnce(node);

    if (node != nullptr)
    {
        return AUDIO_Generator::readAudioOnce(node);
    }

    // RLP_LOG_DEBUG("")

    bool returnResult = true;

    QList<DS_NodePtr> nodeList = _controller->session()->getAllNodesByFrame(_cache.endTime()->videoFrame());

    // if no tracks have audio, abort
    bool audioFound = false;
    for (int i = 0; i != nodeList.size(); ++i)
    {
        DS_NodePtr node = nodeList.at(i);
        if (node->hasProperty("audio.format.reader"))
        {
            audioFound = true;
            break;
        }
    }

    if (!audioFound)
    {
        RLP_LOG_VERBOSE("NO AUDIO READER: aborting");
        return false;
    }

    // RLP_LOG_DEBUG(nodeList.size())

    QList< QList<DS_AudioBufferPtr> > allBuffers;

    int maxBufferCount = 0;

    for (int i = 0; i != nodeList.size(); ++i)
    {
        DS_NodePtr node = nodeList.at(i);

        QList<DS_AudioBufferPtr> bufferList;

        if (!(node->hasProperty("audio.format.reader")))
        {
            continue;
        }

        qlonglong videoFrameOffset = _controller->session()->getNodeFrameIndex(node);

        // NOTE: UNALLOCATED
        //
        DS_AudioBufferPtr ab(new DS_AudioBuffer(this));

        AUDIO_TimePtr time = _cache.endTime()->copy();
        time->setVideoFrameOffset(videoFrameOffset);

        node->readAudio(time, ab);

        int nb_frames = _cache.audioFormat().framesForBytes(ab->getDataSize()); //  / _format.bytesPerFrame();
        float nb_buffers_fl = (float)nb_frames / (float)_cache.samplesPerBuffer();
        int nb_buffers = int(nb_buffers_fl);

        // Add remainder into a last buffer
        if ((nb_buffers_fl - nb_buffers) > 0) {
            nb_buffers++;
        }

        if (nb_buffers > maxBufferCount)
        {
            maxBufferCount = nb_buffers;
        }


        int nodeReadBytesRemaining = ab->getDataSize();
        int nodeReadBufSize = 0;

        for (int x=0; x != nb_buffers; ++x) {


            DS_AudioBufferPtr buf(new DS_AudioBuffer(this, _audioInfo));
            buf->reallocate();

            // RLP_LOG_DEBUG("samples:" << buf->samples() << "channels:" << buf->channels() << "bytesPerSample:" << buf->bytesPerSample())

            int bufSize = buf->getDataBytes();
            if (bufSize > nodeReadBytesRemaining) {
                bufSize = nodeReadBytesRemaining;
                buf->setDataSize(nodeReadBytesRemaining);
            }

            // RLP_LOG_DEBUG("BUFFER SPLIT " << x << " with " << buf->getDataBytes() << " bytes");


            memcpy(
                buf->data(),
                (char *)ab->data() + nodeReadBufSize,
                bufSize);


            bufferList.append(buf);
            
            // _cache.append(buf);

            nodeReadBytesRemaining -= buf->getDataBytes();
            nodeReadBufSize += bufSize;

        }

        ab->free();

        int audioReadResult = ab->getBufferMetadata()->value("audio.read_result").toInt();
        if (audioReadResult == (int)DS_Buffer::READ_EOF_OK)
        {
            // RLP_LOG_WARN("premature end of file detected?");
            // try advancing to the next frame?
            _cache.endTime()->setVideoFrame(_cache.endTime()->videoFrame() + 1);
            // returnResult = false;
        }
        else if (audioReadResult == (int)DS_Buffer::READ_NONE)
        {
            return false;
        }

        allBuffers.append(bufferList);

    }


    //
    // MIX
    //


    for (int x = 0; x < maxBufferCount; ++x)
    {
        DS_AudioBufferPtr buf(new DS_AudioBuffer(this, _audioInfo));
        buf->reallocate();

        int16_t* dest = (int16_t*)buf->data();

        int maxDataSize = 0;


        for (int ni = 0; ni != allBuffers.size(); ++ni)
        {
            QList<DS_AudioBufferPtr> bl = allBuffers.at(ni);
            if (x < bl.size())
            {
                // std::cout << x << "/" << maxBufferCount << " nodeidx:" << ni << std::endl;

                DS_AudioBufferPtr b = bl.at(x);
                int16_t* bufData = (int16_t*)b->data();

                if (maxDataSize < b->getDataSize())
                    maxDataSize = b->getDataSize();

                for (int si=0; si != b->samples(); ++si)
                {
                    dest[(si * 2)] += bufData[(si * 2)];
                    dest[(si * 2) + 1] += bufData[(si * 2) + 1];
                }
                
                b->free();
            }
        }
        

        buf->setDataSize(maxDataSize);

        _cache.append(buf);
    }

    return returnResult;

}
