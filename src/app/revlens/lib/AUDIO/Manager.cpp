//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//


#include "RlpRevlens/AUDIO/Manager.h"
#include "RlpRevlens/AUDIO/Generator.h"
#include "RlpRevlens/AUDIO/Mixer.h"

#include "RlpRevlens/MEDIA/Plugin.h"


RLP_SETUP_LOGGER(revlens, AUDIO, Manager)

AUDIO_Manager::AUDIO_Manager(DS_ControllerPtr controller) :
    _controller(controller),
    _audioOutput(nullptr),
    _enabled(true),
    _volume(1.0)
{
    #ifdef SCAFFOLD_WASM
    RLP_LOG_WARN("WASM/IOS detected, disabling")
    _enabled = false;
    #endif

    _format = AUDIO_Generator::makeAudioFormat();

    RLP_LOG_DEBUG(controller->session()->name());

    _mixer = new AUDIO_Mixer(_format, controller);

    // connect(
    //     qobject_cast<CNTRL_MainController*>(_controller.get()),
    //     &CNTRL_MainController::sessionChanged,
    //     this,
    //     &AUDIO_Manager::onSessionChanged
    // );
    connect(
        _controller.get(),
        SIGNAL(sessionChanged(DS_SessionPtr)),
        this,
        SLOT(onSessionChanged(DS_SessionPtr))
    );

    onSessionChanged(controller->session());
}


AUDIO_Manager::~AUDIO_Manager()
{
}


void
AUDIO_Manager::setEnabled(bool isEnabled)
{
    RLP_LOG_DEBUG(isEnabled);

    _enabled = isEnabled;
}


void
AUDIO_Manager::fillBuffer()
{
    RLP_LOG_ERROR("DISABLED")
}


AUDIO_Mixer*
AUDIO_Manager::mixer()
{
    return _mixer;
}


void
AUDIO_Manager::setTargetFrameRate(float frameRate)
{
    if (!_enabled) {
        // RLP_LOG_WARN("DISABLED - doing nothing");
        return;
    }

     _mixer->setTargetFrameRate(frameRate);
}


float
AUDIO_Manager::getTargetFrameRate()
{
    return _mixer->getTargetFrameRate();
}


void
AUDIO_Manager::updateToFrame(qlonglong frame, bool recenter)
{
    // RLP_LOG_DEBUG(frame);
    // return;

    if (!_enabled) {
        // RLP_LOG_WARN("DISABLED - doing nothing");
        return;
    }

    if (_audioOutput == nullptr) {
        RLP_LOG_ERROR("No audio output!");
        return;
    }

    suspend();

    // RLP_LOG_DEBUG("Restting Audio Buffer")

    if (recenter)
    {
        _audioOutput->reset(); // clear the internal buffer for the audio sink
    }

    _mixer->updateToFrame(frame, recenter);
}


/*
void
AUDIO_Manager::stop(qlonglong frame)
{
    updateToFrame(frame);
}
*/

void
AUDIO_Manager::setVolume(float volume)
{
    if (_audioOutput == nullptr) {
        RLP_LOG_ERROR("No audio output!")
        return;
    }

    if (volume != _audioOutput->volume()) {

        RLP_LOG_DEBUG(volume);
        _volume = volume;
        _audioOutput->setVolume(volume);
    }
}


void
AUDIO_Manager::mute()
{
    RLP_LOG_DEBUG("")

    _volume = _audioOutput->volume();
    _audioOutput->setVolume(0.0);
}


void
AUDIO_Manager::unmute()
{
    RLP_LOG_DEBUG("setting volume to" << _volume)

    _audioOutput->setVolume(_volume);
}


void
AUDIO_Manager::initializeAudio()
{
    if (!_enabled) {
        RLP_LOG_WARN("AUDIO DISABLED, skipping creation of audio output device");
        return;
    }

    RLP_LOG_DEBUG("");

    createAudioOutput();
}


void
AUDIO_Manager::createAudioOutput()
{
    RLP_LOG_DEBUG("")

    delete _audioOutput;

    QMediaDevices devices(this);
    const QAudioDevice device = devices.defaultAudioOutput();

    _audioOutput = 0;
    _audioOutput = new QAudioSink(device, _format);
    _audioOutput->setBufferSize(BUFFER_SIZE);
    _audioOutput->setVolume(1.0);

    connect(_audioOutput, SIGNAL(notify()), SLOT(notified()));
    connect(_audioOutput, SIGNAL(stateChanged(QAudio::State)), SLOT(handleStateChanged(QAudio::State)));

    RLP_LOG_DEBUG("output buffer size: " << _audioOutput->bufferSize());
    // RLP_LOG_DEBUG("AUDIO_Manager::createAudioOutput() : bytes per frame: " << _format.bytesPerFrame());


    _mixer->initAudioOutput();

    emit mgrAudioReady();
}


void
AUDIO_Manager::notified()
{
    /*
    RLP_LOG_DEBUG("AUDIO_Manager::notified()");
    RLP_LOG_DEBUG("bytesFree = " << _audioOutput->bytesFree()
               << ", " << "elapsedUSecs = " << _audioOutput->elapsedUSecs()
               << ", " << "processedUSecs = " << _audioOutput->processedUSecs());
   */
}


void
AUDIO_Manager::handleStateChanged(QAudio::State state)
{
    // RLP_LOG_DEBUG(int(state));

    if (state == QAudio::SuspendedState)
    {

        // RLP_LOG_DEBUG("SUSPENDED");

        // _audioOutput->setVolume(0.0); // to prevent pops on pause/resume

    } else if (state == QAudio::ActiveState)
    {
        //RLP_LOG_DEBUG("ACTIVE");

    } else if (state == QAudio::StoppedState)
    {
        // RLP_LOG_DEBUG("STOPPED");
        // _audioOutput->setVolume(0.0); // to prevent pops on pause/resume

    } else if (state == QAudio::IdleState)
    {
        RLP_LOG_DEBUG("IDLE");

    } else
    {
        // RLP_LOG_DEBUG("UNKNOWN STATE");
    }
}


void
AUDIO_Manager::suspend()
{
    if (!_enabled) {
        RLP_LOG_WARN("DISABLED - doing nothing");
        return;
    }

    // RLP_LOG_DEBUG("AUDIO_Manager::suspend(): saving volume " << _volume);

    _volume = _audioOutput->volume();
    _audioOutput->setVolume(0.0); // to prevent pops on pause/resume

    if (_audioOutput->state() == QAudio::ActiveState)
    {
        // RLP_LOG_DEBUG("audio active state, running suspend");

        _audioOutput->suspend();

    } else if (_audioOutput->state() == QAudio::StoppedState)
    {
        // RLP_LOG_DEBUG("audio stopped state, doing nothing (WARNING)");

    } else if (_audioOutput->state() == QAudio::SuspendedState)
    {
        // RLP_LOG_DEBUG("audio suspended state, doing nothing (WARNING)");

    } else
    {
        RLP_LOG_WARN("unknown state at suspend(): " << (int)_audioOutput->state());
    }

    _audioOutput->setVolume(_volume);
}


void
AUDIO_Manager::resume()
{
    if (!_enabled)
    {
        RLP_LOG_WARN("DISABLED - doing nothing");
        return;
    }

    if (_audioOutput == nullptr)
    {
        RLP_LOG_ERROR("Invalid audio output!")
        return;
    }

    _audioOutput->setVolume(0.0); // to prevent pops on pause/resume

    // RLP_LOG_DEBUG("buffer size: " << _audioOutput->bufferSize());

    if (_audioOutput->state() == QAudio::ActiveState)
    {

        RLP_LOG_WARN("audio active state, doing nothing");

    } else if (_audioOutput->state() == QAudio::StoppedState)
    {

        RLP_LOG_DEBUG("audio stopped state, running start");

        _audioOutput->start(_mixer);

    } else if (_audioOutput->state() == QAudio::SuspendedState)
    {
        RLP_LOG_DEBUG("audio suspended state, running resume");
        _audioOutput->resume();
    }

    _audioOutput->setVolume(_volume);
}


void
AUDIO_Manager::scheduleGenerateWaveform(DS_Node* node)
{
    RLP_LOG_DEBUG("");

    #ifdef SCAFFOLD_WASM
    RLP_LOG_WARN("Generating waveform bypassed for load time")
    return;
    #endif


    // _cacheController = DS_ControllerPtr(new MEDIA_DiskCacheController(_uuid.toString()));

    AUDIO_GenerateWaveformCacheTask* generateWaveformTask = new AUDIO_GenerateWaveformCacheTask(node);
    MEDIA_Plugin* plugin = node->getPropertiesPtr()->value("audio.plugin").value<MEDIA_Plugin*>();

    if (plugin != nullptr)
    {
        plugin->getReadThreadPool(DS_Node::DATA_AUDIO)->start(generateWaveformTask);
    }

}


void
AUDIO_Manager::onSessionChanged(DS_SessionPtr session)
{
    RLP_LOG_DEBUG("")

    connect(
        session.get(),
        &DS_Session::mediaAdded,
        this,
        &AUDIO_Manager::onMediaAdded
    );
}


void
AUDIO_Manager::onMediaAdded(DS_NodePtr node, QString uuidStr, qlonglong frameNum, qlonglong mediaFrameCount)
{
    RLP_LOG_DEBUG(uuidStr << frameNum << mediaFrameCount)

    scheduleGenerateWaveform(node.get());
}


void
AUDIO_Manager::onNodeDataReady(QVariantMap data)
{
    QString dataType = data.value("data_type").toString();
    if (dataType == "audio_source")
    {
        DS_Node* node = data.value("node").value<DS_Node*>();
        scheduleGenerateWaveform(node);
    }
}
