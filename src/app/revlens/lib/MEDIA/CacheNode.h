//
//  Copyright (c) 2014 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_MEDIA_CACHE_NODE_H
#define REVLENS_MEDIA_CACHE_NODE_H

#include "RlpRevlens/MEDIA/Global.h"

#include "RlpCore/LOG/Logging.h"
#include "RlpRevlens/DS/Node.h"
#include "RlpRevlens/DS/Session.h"
#include "RlpRevlens/DS/Track.h"
#include "RlpRevlens/DS/ControllerBase.h"
#include "RlpRevlens/AUDIO/Generator.h"
#include "RlpRevlens/AUDIO/Cache.h"



class MEDIA_ThumbnailCacheTask;

class REVLENS_MEDIA_API MEDIA_CacheNode : public DS_Node {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    enum ThumbCacheState {
        THUMB_CACHE_UNINITIALIZED,
        THUMB_CACHE_RUNNING,
        THUMB_CACHE_READY,
        THUMB_CACHE_DISABLED
    };

    MEDIA_CacheNode(QVariantMap* properties, DS_NodePtr nodeIn);

    void readVideo(
        DS_TimePtr timeInfo,
        DS_BufferPtr destFrame,
        bool postUpdate = true
    );

    void readAudio(
        DS_TimePtr timeInfo,
        DS_BufferPtr destBuffer
    );

    void scheduleReadTask(
        DS_TimePtr timeInfo,
        DS_BufferPtr destFrame,
        NodeDataType dataType,
        bool optional = false
    );

    ThumbCacheMap* thumbCache() { return _thumbCache; }
    void setThumbCacheState(ThumbCacheState state);

public slots:
    void continueThumbnailGen();


private: // methods

    void scheduleThumbnailGen();
    ThumbCacheState thumbCacheState();


private: // members

    // Thumbnail cache
    //
    ThumbCacheMap* _thumbCache;
    ThumbCacheState _thumbCacheState;
    MEDIA_ThumbnailCacheTask* _thumbCacheTask;
    QThreadPool _thpool;
    QTimer _ttimer;
    QMutex _tlock;

};

class REVLENS_MEDIA_API MEDIA_CacheReadTask : public QRunnable {

public:
    MEDIA_CacheReadTask(
        MEDIA_CacheNode* cacheNode,
        DS_TimePtr timeInfo,
        DS_BufferPtr destFrame
    );

    void run();

private:
    MEDIA_CacheNode* _cacheNode;
    DS_TimePtr _timeInfo;
    DS_BufferPtr _destFrame;
};


class REVLENS_MEDIA_API MEDIA_ThumbnailCacheTask : public QRunnable {

public:
    RLP_DEFINE_LOGGER

    MEDIA_ThumbnailCacheTask(
        MEDIA_CacheNode* cacheNode
    );

    void run();

    qlonglong fnum() { return _fnum; }

private:
    MEDIA_CacheNode* _cacheNode;
    DS_NodePtr _thumbReader;
    DS_TimePtr _time;
    DS_FrameBufferPtr _fbuf;
    qlonglong _fnum;

};

#endif