
#ifndef REVLENS_AUDIO_WAVEFORM_H
#define REVLENS_AUDIO_WAVEFORM_H

#include "RlpRevlens/AUDIO/Global.h"

#include "RlpRevlens/DS/AudioFormat.h"
#include "RlpRevlens/DS/Node.h"

typedef QList<QPair<float, float> > WaveformDataList;
typedef QList<QPair<float, float> >::iterator WaveformDataListIterator;


class AUDIO_Waveform : public QObject {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    AUDIO_Waveform(QByteArray* audioData, float frameRate, qlonglong frameCount);

    // QImage drawWaveformForVideoFrame(qlonglong videoFrame);

    QPixmap* drawWaveform(float pixelsPerFrame=10.0);


private:
    QByteArray* _audioData;
    qlonglong _frameCount;
    DS_AudioFormat _audioFormat;

};


class AUDIO_GenerateWaveformTask : public QRunnable {

public:
    RLP_DEFINE_LOGGER

    AUDIO_GenerateWaveformTask(DS_NodePtr node, float pixelsPerFrame);

    void run();


private:
    DS_NodePtr _node;
    float _pixelsPerFrame;

};

#endif
