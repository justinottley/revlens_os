//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_AUDIO_WAVEFORM_H
#define REVLENS_AUDIO_WAVEFORM_H

#include "RlpRevlens/DS/ControllerBase.h"
#include "RlpRevlens/AUDIO/Global.h"

typedef QList<QPair<float, float> > WaveformDataList;
typedef QList<QPair<float, float> >::iterator WaveformDataListIterator;

class AUDIO_Cache;

class REVLENS_AUDIO_API AUDIO_WaveformCache : public QObject {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER
    
    
    AUDIO_WaveformCache(AUDIO_Cache* cache) { _audioCache = cache; }

    void initAudioWaveformData(QList<long> &audioList, float pixelsPerFrame=10.0);

public slots:
    
    void clear();
    void drawWaveform(QPixmap* pixmap, long startFrame, long numFrames, float pixelsPerFrame=10.0);
    long size() { return _waveformCache.size(); }
private:
    
    AUDIO_Cache* _audioCache;
    QHash<long, WaveformDataList*> _waveformCache;
    
};

class REVLENS_AUDIO_API AUDIO_WaveformCacheController : public DS_ControllerBase {
    Q_OBJECT

public:
    AUDIO_WaveformCacheController(QString sessionName);
    

public slots:

    DS_SessionPtr session() { return _session; }
    DS_PlaybackMode* playbackMode() { return _playbackMode; }

private:

    DS_SessionPtr _session;
    DS_PlaybackMode* _playbackMode;
};


class REVLENS_AUDIO_API AUDIO_GenerateWaveformCacheTask : public QRunnable {

public:
    RLP_DEFINE_LOGGER

public:

    AUDIO_GenerateWaveformCacheTask(DS_Node* node);

    void runGenerateWaveform(DS_ControllerPtr cacheController, QVariantMap* nodeProperties, DS_NodePtr audioReader);
    void run();

private:

    DS_Node* _node;

};


#endif