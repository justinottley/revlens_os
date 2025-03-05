//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef EXTREVLENS_RLFFMPEG_VIDEOREADER_H
#define EXTREVLENS_RLFFMPEG_VIDEOREADER_H


#include "RlpExtrevlens/RLFFMPEG/Global.h"

#include "RlpExtrevlens/RLFFMPEG/ReaderBase2.h"

#include "RlpRevlens/DS/Node.h"
#include "RlpRevlens/MEDIA/ReadEvent.h"

extern "C" {

#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

}


class RLFFMPEG_ThumbnailCacheTask;

class EXTREVLENS_RLFFMPEG_API RLFFMPEG_VideoReader : public DS_Node {
    Q_OBJECT

public:
    enum ThumbCacheState {
        THUMB_CACHE_UNINITIALIZED,
        THUMB_CACHE_RUNNING,
        THUMB_CACHE_READY,
        THUMB_CACHE_DISABLED
    };

public:
    RLP_DEFINE_LOGGER

    RLFFMPEG_VideoReader(QVariantMap* properties);

    void scheduleReadTask(
        DS_TimePtr timeInfo,
        DS_BufferPtr destBuffer,
        DS_Node::NodeDataType dataType,
        bool optional = false);

    void readVideo(DS_TimePtr timeInfo,
                   DS_BufferPtr destBuffer,
                   bool postUpdate = true);

    uint64_t getDts(long frameNum);

    void resetAll();

    ThumbCacheMap* thumbCache() { return _thumbCache; }
    void setThumbCacheState(ThumbCacheState state);

public slots:
    void continueThumbnailGen();

protected:

    virtual void init();

    void initializeScalars();
    int initializeCodecContext();
    void initializeDestFormatContext();

    bool openMedia();
    void closeMedia();

    RLFFMPEG_PacketReadResult seekToFrame(qlonglong frameNum);

    bool getFrame(AVPacket* pkt, AVFrame* frame);
    RLFFMPEG_PacketReadResult readNextPacket();

    void scheduleThumbnailGen();
    ThumbCacheState thumbCacheState();

protected:

    // Base
    //
    AVMediaType _AVMEDIA_TYPE;

    AVFormatContext* _fmtCtx;
    AVCodecContext* _streamDecCtx; 

    int _streamIdx;

    QString _srcFilePath;
    QString _readerType;

    float _frameRate;

    uint64_t _ptsOffset;
    uint64_t _ptsIncrement;
    uint64_t _ptsMax;

    qlonglong _lastVideoFrame;

    // Video
    //
    SwsContext* _swsCtx;
    int _rgbLineSize[4];

    RLFFMPEG_PacketReadResult _lastReadResult;

    bool _flushMode;

    QMutex _lock;

    ThumbCacheMap* _thumbCache;
    ThumbCacheState _thumbCacheState;
    RLFFMPEG_ThumbnailCacheTask* _thumbCacheTask;
    QThreadPool _thpool;
    QTimer _ttimer;
};


#endif
