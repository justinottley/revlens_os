//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef EXTREVLENS_RLFFMPEG_PLUGIN_H
#define EXTREVLENS_RLFFMPEG_PLUGIN_H

#include "RlpExtrevlens/RLFFMPEG/Global.h"

#include "RlpRevlens/DS/Node.h"
#include "RlpRevlens/DS/FrameBuffer.h"
#include "RlpRevlens/MEDIA/Plugin.h"


class AVFormatContext;

class RLFFMPEG_VideoReader;
class RLFFMPEG_AudioReader;

class EXTREVLENS_RLFFMPEG_API RLFFMPEG_Plugin : public MEDIA_Plugin {
    Q_OBJECT
    
public:

    RLP_DEFINE_LOGGER
    
    
    RLFFMPEG_Plugin();
    
    DS_NodePtr createVideoReader(QVariantMap* properties);
    DS_NodePtr createAudioReader(QVariantMap* properties);
    
private:
    
    bool identifyVideo(QString srcFilePath,
                       QVariantMap& mediaIn,
                       QVariantList* result);

    bool identifyAudio(QString srcFilePath,
                       QVariantMap& mediaIn,
                       QVariantList* result);

    bool identifyNullVideoForAudioOnly(QVariantMap& mediaIn, QVariantList* result);

public slots:
    
    QVariantList identifyMedia(QVariantMap mediaInput, DS_Node::NodeDataType dataType=DS_Node::DATA_UNKNOWN);
    QString getSettingsUI() { return QString("rlplug_ffmpeg.panel.settings"); }

};


Q_DECLARE_METATYPE(RLFFMPEG_Plugin *)



class EXTREVLENS_RLFFMPEG_API RLFFMPEG_VideoReadTask : public QRunnable {

public:

    RLFFMPEG_VideoReadTask(
        RLFFMPEG_VideoReader* reader,
        DS_TimePtr timeInfo,
        DS_BufferPtr destFrame
    );

    void run();

private:
    
    RLFFMPEG_VideoReader* _reader;
    DS_TimePtr _timeInfo;
    DS_BufferPtr _destFrame;
};


class EXTREVLENS_RLFFMPEG_API RLFFMPEG_AudioReadTask : public QRunnable {

public:

    RLFFMPEG_AudioReadTask(
        RLFFMPEG_AudioReader* reader,
        DS_TimePtr timeInfo,
        DS_BufferPtr destFrame
    );

    void run();

private:
    
    RLFFMPEG_AudioReader* _reader;
    DS_TimePtr _timeInfo;
    DS_BufferPtr _destFrame;
};


class EXTREVLENS_RLFFMPEG_API RLFFMPEG_ThumbnailCacheTask : public QRunnable {

public:
    RLP_DEFINE_LOGGER

    RLFFMPEG_ThumbnailCacheTask(
        RLFFMPEG_VideoReader* reader
    );

    void run();

    qlonglong fnum() { return _fnum; }

private:
    RLFFMPEG_VideoReader* _reader;
    RLFFMPEG_VideoReader* _thumbReader;
    DS_TimePtr _time;
    DS_FrameBufferPtr _fbuf;
    qlonglong _fnum;

};

#endif

