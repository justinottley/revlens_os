
#include "RlpRevlens/AUDIO/Handler.h"
#include "RlpRevlens/AUDIO/Waveform.h"


RLP_SETUP_LOGGER(revlens, AUDIO, Handler)


AUDIO_Handler::AUDIO_Handler(DS_ControllerBase* controller):
    _controller(controller),
    _audioSink(nullptr),
    _audioOutput(nullptr),
    _scrubbingEnabled(false),
    _muted(false),
    _volume(1.0)
{
    RLP_LOG_DEBUG("")


    QAudioFormat format;
    format.setSampleRate(AUDIO_DATA_SAMPLE_RATE_HZ);
    format.setSampleFormat(QAudioFormat::Int16);
    format.setChannelConfig(QAudioFormat::ChannelConfigStereo);
    // format.setCodec("audio/pcm");

    _audioFormat = format;

}


void
AUDIO_Handler::init()
{
    RLP_LOG_DEBUG("")

    QMediaDevices devices(this);
    const QAudioDevice device = devices.defaultAudioOutput();

    RLP_LOG_DEBUG("Audio Buffer Size:" << AUDIO_SINK_BUFFER_SIZE)

    _audioSink = new QAudioSink(device, _audioFormat);
    _audioSink->setBufferSize(AUDIO_SINK_BUFFER_SIZE);
    _audioSink->setVolume(1.0);

    _audioOutput = new AUDIO_OutputDevice(_controller);
    connect(
        _audioOutput,
        &AUDIO_OutputDevice::requestAudioStop,
        this,
        &AUDIO_Handler::stop
    );

    _audioOutput->open(QIODevice::ReadOnly);
}

void
AUDIO_Handler::setTargetFrameRate(float frameRate)
{
    RLP_LOG_DEBUG(frameRate)

    _audioOutput->setTargetFrameRate(frameRate);
}


float
AUDIO_Handler::getTargetFrameRate()
{
    return _audioOutput->getTargetFrameRate();
}


void
AUDIO_Handler::mute()
{
    RLP_LOG_DEBUG("")

    _muted = true;
    _volume = _audioSink->volume();

    _audioSink->setVolume(0.0);
    _audioSink->suspend();
    _audioSink->reset();
}


float
AUDIO_Handler::unmute()
{
    if (_volume == 0.0)
    {
        _volume = 1.0;
    }

    RLP_LOG_DEBUG(_volume)

    _muted = false;

    _audioOutput->updateToFrame(_controller->currentFrameNum());

    _audioSink->start(_audioOutput);
    _audioSink->setVolume(_volume);
    

    return _volume;
}


float
AUDIO_Handler::volume()
{
    return _audioSink->volume();
}


void
AUDIO_Handler::setVolume(float volume)
{
    // RLP_LOG_DEBUG(volume)

    _volume = volume;
    _audioSink->setVolume(volume);

}


void
AUDIO_Handler::resume()
{
    // RLP_LOG_DEBUG("")

    _audioSink->reset();
    _audioSink->start(_audioOutput);
}


void
AUDIO_Handler::stop(qlonglong frameNum)
{
    // RLP_LOG_DEBUG(frameNum)

    _audioSink->suspend();
    _audioSink->reset();
    _audioOutput->updateToFrame(frameNum);
}


void
AUDIO_Handler::updateToFrame(qlonglong frameNum)
{
    RLP_LOG_DEBUG(frameNum)

    _audioOutput->updateToFrame(frameNum);

    if (_scrubbingEnabled)
    {
        _audioOutput->setScrubbing(frameNum, true, true);
    
        _audioSink->reset();
        _audioSink->start(_audioOutput);
    }
}


void
AUDIO_Handler::clearAll()
{
    _audioOutput->clearAll();
}


void
AUDIO_Handler::toggleAudioScrubbing()
{
    _scrubbingEnabled = !_scrubbingEnabled;

    RLP_LOG_DEBUG(_scrubbingEnabled)
}


void
AUDIO_Handler::onScrubStart(qlonglong frameNum)
{
    // RLP_LOG_DEBUG(frameNum);

    _audioSink->suspend();
    _audioSink->reset();
    _audioOutput->updateToFrame(frameNum);

    if (!_scrubbingEnabled)
    {
        return;
    }

    _audioOutput->setScrubbing(frameNum, true);
    _audioSink->start(_audioOutput);
}


void
AUDIO_Handler::onScrubEnd(qlonglong frameNum)
{
    // RLP_LOG_DEBUG(frameNum)

    _audioSink->suspend();
    _audioSink->reset();

    _audioOutput->setScrubbing(frameNum, false);
    _audioOutput->updateToFrame(frameNum);
}


void
AUDIO_Handler::onScrub(qlonglong frameNum)
{
    // RLP_LOG_DEBUG(frameNum)

    _audioOutput->updateToFrame(frameNum);
    _audioOutput->setScrubFrame(frameNum);

    #ifdef __linux__
    if (_scrubbingEnabled)
    {
        _audioSink->reset();
        _audioSink->start(_audioOutput);
    }
    #endif
}
