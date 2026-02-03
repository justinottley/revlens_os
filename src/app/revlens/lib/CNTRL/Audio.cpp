//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include "RlpRevlens/CNTRL/Audio.h"

#include "RlpCore/UTIL/AppGlobals.h"
#include "RlpRevlens/DS/ControllerBase.h"

#include "RlpRevlens/CNTRL/MainController.h"

RLP_SETUP_LOGGER(revlens, CNTRL, Audio)

CNTRL_Audio::CNTRL_Audio(CNTRL_MainController* controller) :
    _controller(controller)
{
    _audioManager = new AUDIO_Manager(DS_ControllerPtr(controller));
    setupConnections();
}


void
CNTRL_Audio::setupConnections()
{
    //
    // NOTE: This is currently running in the main thread..
    //

    RLP_LOG_DEBUG("");


    // TODO FIXME: MOVE THIS i think somewhere else


    connect(_controller,
            SIGNAL(startupReady()),
            this,
            SLOT(onStartupReady()));


    connect(_audioManager->mixer(),
            SIGNAL(bufferFillEvent(QList<qlonglong>)),
            this,
            SLOT(onMgrBufferFillEvent(QList<qlonglong>)));

}


void
CNTRL_Audio::clearAll()
{
    RLP_LOG_DEBUG("");

    _audioManager->mixer()->clear();
    _audioManager->updateToFrame(_controller->currentFrameNum());
}


void
CNTRL_Audio::onStartupReady()
{
    RLP_LOG_DEBUG("");

    #ifdef SCAFFOLD_IOS
    RLP_LOG_WARN("Skipping audio")
    return;
    #endif

    #ifdef SCAFFOLD_WASM
    RLP_LOG_WARN("Skipping audio")
    return;
    #endif


    QVariantMap globals = UTIL_AppGlobals::instance()->globals();

    if (globals.contains("startup.audio_enabled"))
    {

        bool audioEnabled = globals.value("startup.audio_enabled", true).value<bool>();

        RLP_LOG_DEBUG("startup.audio_enabled: " << audioEnabled);
        _audioManager->setEnabled(audioEnabled);

    }

    _audioManager->initializeAudio();
}


void
CNTRL_Audio::mute()
{
    _audioManager->mute();
}


float
CNTRL_Audio::unmute()
{
    float newVolume = _audioManager->unmute();
    return newVolume;
}


float
CNTRL_Audio::toggleMute()
{
    RLP_LOG_DEBUG("")

    float newVolume = _audioManager->toggleMute(); 
    emit volumeChanged(newVolume);

    return newVolume;
}


void
CNTRL_Audio::onMgrAudioReady()
{
    RLP_LOG_DEBUG("");

    emit audioReady();
}


void
CNTRL_Audio::onMgrBufferFillEvent(QList<qlonglong> audioList)
{
    // RLP_LOG_DEBUG("RESENDING SIGNAL num frames:" << audioList.size())

    emit bufferFillEvent(audioList);
}
