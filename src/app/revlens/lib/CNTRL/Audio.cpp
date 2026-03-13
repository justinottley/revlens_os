
#include "RlpRevlens/CNTRL/Audio.h"
#include "RlpRevlens/CNTRL/MainController.h"
#include "RlpRevlens/MEDIA/Plugin.h"

#include "RlpRevlens/AUDIO/Waveform.h"

#include "RlpCore/DS/Flags.h"
#include "RlpCore/UTIL/AppGlobals.h"

RLP_SETUP_LOGGER(revlens, CNTRL, Audio)


CNTRL_Audio::CNTRL_Audio(CNTRL_MainController* controller):
    _controller(controller),
    _audioThread(nullptr),
    _audioHandler(nullptr),
    _enabled(true),
    _muted(false),
    _volume(1.0),
    _frameRate(-1)
{
    RLP_LOG_DEBUG("")

    CoreDs_Flags::registerFlag("audio_data_ready");
    CoreDs_Flags::registerFlag("audio_waveform_ready");

    UTIL_AppGlobals* ag = UTIL_AppGlobals::instance();
    ag->insert("audio.output.bit_depth", AUDIO_DATA_BYTES_PER_SAMPLE);
    ag->insert("audio.output.channel_count", AUDIO_DATA_CHANNEL_COUNT);
    ag->insert("audio.output.sample_rate", AUDIO_DATA_SAMPLE_RATE_HZ);

    if (ag->contains("startup.audio_enabled"))
    {
        bool audioEnabled = ag->value("startup.audio_enabled").toBool();
        _enabled = audioEnabled;
    }

    connect(
        _controller,
        SIGNAL(startupReady()),
        this,
        SLOT(initAudioOutput())
    );

}


void
CNTRL_Audio::initAudioOutput()
{
    RLP_LOG_DEBUG("")

    UTIL_AppGlobals* ag = UTIL_AppGlobals::instance();
    if (ag->contains("startup.audio_enabled"))
    {
        bool audioEnabled = ag->value("startup.audio_enabled").toBool();
        _enabled = audioEnabled;
    }

    if (!_enabled)
    {
        RLP_LOG_WARN("Audio disabled")
        return;
    }

    _audioHandler = new AUDIO_Handler(_controller);

    connect(
        this,
        &CNTRL_Audio::setTargetFrameRateRequest,
        _audioHandler,
        &AUDIO_Handler::setTargetFrameRate
    );

    connect(
        this,
        &CNTRL_Audio::muteRequest,
        _audioHandler,
        &AUDIO_Handler::mute
    );

    connect(
        this,
        &CNTRL_Audio::unmuteRequest,
        _audioHandler,
        &AUDIO_Handler::unmute
    );

    connect(
        this,
        &CNTRL_Audio::setVolumeRequest,
        _audioHandler,
        &AUDIO_Handler::setVolume
    );

    connect(
        this,
        &CNTRL_Audio::resumeRequest,
        _audioHandler,
        &AUDIO_Handler::resume
    );

    connect(
        this,
        &CNTRL_Audio::stopRequest,
        _audioHandler,
        &AUDIO_Handler::stop
    );

    connect(
        this,
        &CNTRL_Audio::updateToFrameRequest,
        _audioHandler,
        &AUDIO_Handler::updateToFrame
    );

    connect(
        this,
        &CNTRL_Audio::clearAllRequest,
        _audioHandler,
        &AUDIO_Handler::clearAll
    );

    connect(
        this,
        &CNTRL_Audio::scrubStartRequest,
        _audioHandler,
        &AUDIO_Handler::onScrubStart
    );

    connect(
        this,
        &CNTRL_Audio::scrubEndRequest,
        _audioHandler,
        &AUDIO_Handler::onScrubEnd
    );

    connect(
        this,
        &CNTRL_Audio::scrubRequest,
        _audioHandler,
        &AUDIO_Handler::onScrub
    );


    connect(
        this,
        &CNTRL_Audio::toggleAudioScrubbingRequest,
        _audioHandler,
        &AUDIO_Handler::toggleAudioScrubbing
    );


    _audioThread = new QThread();

    connect(
        _audioThread,
        &QThread::started,
        _audioHandler, 
        &AUDIO_Handler::init
    );


    _audioHandler->moveToThread(_audioThread);
    _audioThread->start();

    connect(
        _controller->session().get(),
        &DS_Session::mediaAdded,
        this,
        &CNTRL_Audio::onMediaAdded
    );

}


void
CNTRL_Audio::setTargetFrameRate(float frameRate)
{
    RLP_LOG_DEBUG(frameRate)

    _frameRate = frameRate;
    emit setTargetFrameRateRequest(frameRate);
}


float
CNTRL_Audio::getTargetFrameRate()
{
    return _frameRate;
}


void
CNTRL_Audio::mute()
{
    // RLP_LOG_DEBUG("")

    _muted = true;

    emit muteRequest();
}


float
CNTRL_Audio::unmute()
{
    // RLP_LOG_DEBUG("")

    if (_volume == 0.0)
    {
        _volume = 1.0;
    }

    _muted = false;

    emit unmuteRequest();

    return _volume;
}


float
CNTRL_Audio::volume()
{
    return _volume;
}


void
CNTRL_Audio::setVolume(float volume)
{
    // RLP_LOG_DEBUG(volume)

    _volume = volume;

    emit setVolumeRequest(volume);
}


void
CNTRL_Audio::resume()
{
    RLP_LOG_DEBUG("")

    emit resumeRequest();
}


void
CNTRL_Audio::stop(qlonglong frameNum)
{
    RLP_LOG_DEBUG(frameNum)

    emit stopRequest(frameNum);
}


void
CNTRL_Audio::updateToFrame(qlonglong frameNum)
{
    emit updateToFrameRequest(frameNum);
}


void
CNTRL_Audio::clearAll()
{
    emit clearAllRequest();
}


void
CNTRL_Audio::toggleAudioScrubbing()
{
    emit toggleAudioScrubbingRequest();
}


void
CNTRL_Audio::onScrubStart(qlonglong frameNum)
{
    // RLP_LOG_DEBUG(frameNum);

    emit scrubStartRequest(frameNum);
}


void
CNTRL_Audio::onScrubEnd(qlonglong frameNum)
{
    // RLP_LOG_DEBUG(frameNum)

    emit scrubEndRequest(frameNum);
}


void
CNTRL_Audio::onScrub(qlonglong frameNum)
{
    // RLP_LOG_DEBUG(frameNum)

    emit scrubRequest(frameNum);
}


void
CNTRL_Audio::onMediaAdded(
    DS_NodePtr node,
    QString uuidStr,
    qlonglong frameNum,
    qlonglong mediaFrameCount)
{
    // RLP_LOG_DEBUG(node.get())

    if (node->checkFlagByName("audio_waveform_ready"))
    {
        return;
    }

    connect(
        node->graph(),
        &DS_Graph::flagStateChanged,
        this,
        &CNTRL_Audio::onNodeFlagStateChanged
    );
}


void
CNTRL_Audio::onNodeFlagStateChanged(
    QString nodeUuidStr,
    QString flagName,
    bool val)
{
    // RLP_LOG_DEBUG(nodeUuidStr << flagName << val)

    // Heuristic
    //

    if (flagName != "audio_data_ready")
    {
        return;
    }

    DS_NodePtr node = _controller->session()->getNodeByUuid(nodeUuidStr);
    if (node == nullptr)
    {
        RLP_LOG_ERROR("No known node:" << nodeUuidStr)
        return;
    }


    float pixelsPerFrame = int(rint(2000.0 / (float)_controller->session()->frameCount()));
    if (pixelsPerFrame < 1.0)
    {
        pixelsPerFrame = 1.0;
    }

    // LOG_Logging::pprint(*node->getPropertiesPtr());

    MEDIA_Plugin* plugin = nullptr;

    if (node->hasNodeProperty("media.plugin.loader"))
    {
        plugin = node->getProperty<MEDIA_Plugin*>("media.plugin.loader");
    }
    else if (node->hasNodeProperty("audio.plugin"))
    {
        plugin = node->getProperty<MEDIA_Plugin*>("audio.plugin");
    }


    if (plugin != nullptr)
    {
        AUDIO_GenerateWaveformTask* genWaveformTask = new AUDIO_GenerateWaveformTask(
            node, pixelsPerFrame);
        plugin->getReadThreadPool(DS_Node::DATA_AUDIO)->start(genWaveformTask);

    } else
    {
        RLP_LOG_WARN("No plugin available to dispatch waveform gen?")
    }

}
