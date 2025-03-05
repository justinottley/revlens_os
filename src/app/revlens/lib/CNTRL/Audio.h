//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_CNTRL_AUDIO_H
#define REVLENS_CNTRL_AUDIO_H

#include "RlpRevlens/CNTRL/Global.h"

#include "RlpRevlens/AUDIO/Manager.h"
#include "RlpRevlens/AUDIO/Mixer.h"
#include "RlpRevlens/AUDIO/Cache.h"


class CNTRL_MainController;

class REVLENS_CNTRL_API CNTRL_Audio : public QObject {
    Q_OBJECT
    
public:
    
    RLP_DEFINE_LOGGER
    
    
    CNTRL_Audio(CNTRL_MainController* controller);
    
    AUDIO_Manager* mgr() { return _audioManager; }

public slots:
    
    void resume() { _audioManager->resume(); }
    void stop(qlonglong frame) { _audioManager->suspend(); }
    
    bool enabled() { return _audioManager->enabled(); }
    void setEnabled(bool isEnabled) { _audioManager->setEnabled(isEnabled); }
    
    void mute() { _audioManager->mute(); }
    void unmute() { _audioManager->unmute(); }

    void updateToFrame(qlonglong frameNum, bool recenter=false) { _audioManager->updateToFrame(frameNum, recenter); }
    void clearAll();
    
    void setVolume(float volume) { _audioManager->setVolume(volume); }
    
    void fillBuffer() { _audioManager->fillBuffer(); }
    
    AUDIO_Cache* cache() { 
        return _audioManager->mixer()->cache();
    }
    
    void setTargetFrameRate(float frameRate) { _audioManager->setTargetFrameRate(frameRate); }
    float getTargetFrameRate(){ return _audioManager->getTargetFrameRate(); }

    void onScrubEnd(qlonglong frameNum) { _audioManager->updateToFrame(frameNum, /*recenter=*/true); }
    void onMgrAudioReady();
    void onMgrBufferFillEvent(QList<qlonglong> audioList);
    void onStartupReady();

    
signals:

    // void stopAudio(qlonglong frame);
    void audioReady();
    
    void bufferFillEvent(QList<qlonglong>& frameList);
    
    
private slots:
    
    void setupConnections();
    
    
private:
    
    CNTRL_MainController* _controller;
    
    // QThread _audioThread;
    
    AUDIO_Manager* _audioManager;
    float _lastVolume;
    
};

#endif

