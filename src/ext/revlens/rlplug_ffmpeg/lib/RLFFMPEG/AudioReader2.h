//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef EXTREVLENS_RLFFMPEG_AUDIOREADER_H
#define EXTREVLENS_RLFFMPEG_AUDIOREADER_H

#include "RlpExtrevlens/RLFFMPEG/Global.h"
#include "RlpExtrevlens/RLFFMPEG/ReaderBase2.h"

#include "RlpRevlens/DS/Node.h"
#include "RlpRevlens/DS/AudioBuffer.h"

extern "C" {

#include <libswresample/swresample.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/channel_layout.h>

}



class EXTREVLENS_RLFFMPEG_API RLFFMPEG_AudioReader : public DS_Node {
    Q_OBJECT

public:

    RLP_DEFINE_LOGGER


    RLFFMPEG_AudioReader(QVariantMap* properties);

    void scheduleReadTask(
        DS_TimePtr timeInfo,
        DS_BufferPtr destBuffer,
        DS_Node::NodeDataType dataType,
        bool optional = false);


    void readAudio(DS_TimePtr timeInfo, DS_BufferPtr destBuffer);

    uint64_t getPts(long frameNum); // TODO FIXME: NOT USED?

    void resetAll();

public slots:

    /*
    int outputChannelCount() {
        return _properties->value("audio.output.channels").value<int>();
    }

    int outputSampleRate() {
        return _properties->value("audio.output.sampleRate").value<int>();
    }

    int outputBitdepth() {
        return _properties->value("audio.output.bitdepth").value<int>();
    }
    */


private:

    void initializeScalars();
    int initializeCodecContext();
    void initializeDestFormatContext();

    void convertToDestFormat();

    bool openMedia();
    void closeMedia();

    RLFFMPEG_PacketReadResult seekToFrame(long frameNum);

    bool getFrame(AVPacket* pkt, AVFrame* frame);
    RLFFMPEG_PacketReadResult readNextPacket();


private:
    int _outputBitDepth;
    int _outputChannelCount;
    int _outputSampleRate;

    DS_AudioBufferPtr _emptyAudioBuffer;

    // Base
    //
    AVMediaType _AVMEDIA_TYPE;

    AVFormatContext* _fmtCtx;
    AVCodecContext* _streamDecCtx;


    AVFrame* _frame;


    int _streamIdx;

    QString _srcFilePath;
    QString _readerType;

    // float _frameRate;

    uint64_t _ptsOffset;
    uint64_t _ptsIncrement;
    uint64_t _ptsMax;

    int _sampleIncrement;



    // Audio
    //
    SwrContext* _swrCtx;

    double _timeMultiplier;

    float _audioStartVideoFrame;


    double _inSampleRate;

    char _errStr[AV_ERROR_MAX_STRING_SIZE];

    QMutex _lock;

};


#endif
