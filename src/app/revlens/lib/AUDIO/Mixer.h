
#ifndef REVLENS_AUDIO_MIXER_H
#define REVLENS_AUDIO_MIXER_H

#include "RlpRevlens/AUDIO/Global.h"

#include "RlpCore/UTIL/ThreadPool.h" // wasm compat

#include "RlpRevlens/AUDIO/Generator.h"

#include "RlpRevlens/DS/ControllerBase.h"

class AUDIO_Mixer;

class REVLENS_AUDIO_API AUDIO_FillBufferTask : public QRunnable {

public:
    RLP_DEFINE_LOGGER

public:

    AUDIO_FillBufferTask(AUDIO_Mixer* mixer);

    void run();

private:
    AUDIO_Mixer* _mixer;

};


class REVLENS_AUDIO_API AUDIO_Mixer : public AUDIO_Generator {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    AUDIO_Mixer(const QAudioFormat& format, DS_ControllerPtr controller);

    void initAudioOutput();

    bool readAudioOnce(DS_NodePtr node = nullptr);

    bool fillBuffer(); // submit to separate thread
    bool _fillBufferImpl(); // actual implementation


public slots:

    // void setTargetFrameRate(float frameRate, bool doFillBuffer=true);
    // bool fillBuffer();


private:
    QThreadPool _tp;

};

#endif