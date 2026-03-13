

#include "RlpRevlens/AUDIO/AudioReadAllTask.h"

#include "RlpRevlens/DS/AudioBuffer.h"
#include "RlpRevlens/DS/Time.h"

RLP_SETUP_LOGGER(revlens, AUDIO, ReadAllTask)

AUDIO_ReadAllTask::AUDIO_ReadAllTask(
    DS_NodePtr node):
    _node(node)
{
}


void
AUDIO_ReadAllTask::run()
{

    // return;

    float nodeFrameRate = _node->getProperty<float>("video.frame_rate");
    qlonglong frameCount = _node->getProperty<qlonglong>("video.frame_count");

    // RLP_LOG_DEBUG("frame rate:" << nodeFrameRate << "frame count:" << frameCount)
    DS_AudioFormat f(nodeFrameRate);
    qint64 maxAudioBytes = f.bytePositionForFrame(frameCount);

    DS_TimePtr time = DS_TimePtr(new DS_Time(nodeFrameRate));
    time->setVideoFrame(1);

    // AUDIO_TimePtr time = AUDIO_TimePtr(new AUDIO_Time(_frameRate, _samplesPerSec, _bytesPerSample));
    

    QByteArray* result = new QByteArray();

    int bytesRead = 0;
    while (true)
    {
        DS_AudioBufferPtr ab(new DS_AudioBuffer(_node.get()));

        _node->readAudio(time, ab); 

        QVariantMap* abMd = ab->getBufferMetadata();
        int audioReadResult = abMd->value("audio.read_result").toInt();
        if (audioReadResult == (int)DS_Buffer::READ_EOF_OK)
        {
            RLP_LOG_DEBUG("End of file, Done")
            break;
        }
        else if (audioReadResult == (int)DS_Buffer::READ_NONE)
        {
            RLP_LOG_DEBUG("Read error, break")
            break;
        }

        if (abMd->contains("return_status"))
        {
            RLP_LOG_DEBUG("Invalid read, break")
            break;
        }

        result->append((char *)ab->data(), ab->getDataSize());
        bytesRead += ab->getDataSize();

        if (bytesRead > maxAudioBytes)
        {
            RLP_LOG_WARN("Audio too long for video, truncating")
            break;
        }
    }

    // RLP_LOG_DEBUG("DONE:" << result->size() << "maxAudioBytes:" << maxAudioBytes)

    if (result->size() < maxAudioBytes)
    {
        RLP_LOG_WARN("Audio shorter than video, padding")
        int remainderBytes = maxAudioBytes - result->size();
        result->append('\0', remainderBytes);
    }


    QVariantMap md;

    QVariant v;
    v.setValue(result);
    QVariantMap props;
    props.insert("audio.data", v);

    QVariantMap flags;
    flags.insert("audio_data_ready", true);

    md.insert("props", props);
    md.insert("flags", flags);
    md.insert("evt_name", "audio_data_ready");

    // RLP_LOG_DEBUG("Emitting audio data ready to" << _node.get() << _node->graph())

    _node->graph()->emitDataReady("update_properties", md);
}