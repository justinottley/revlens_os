//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//


#ifndef REVLENS_VIDEO_LOOKAHEADCACHE_H
#define REVLENS_VIDEO_LOOKAHEADCACHE_H


#include "RlpCore/LOG/Logging.h"

#include "RlpRevlens/DS/PlaybackMode.h"
#include "RlpRevlens/DS/FrameBuffer.h"
#include "RlpRevlens/DS/Session.h"
#include "RlpRevlens/DS/ControllerBase.h"
#include "RlpRevlens/MEDIA/ReadEvent.h"
#include "RlpRevlens/MEDIA/Plugin.h"
#include "RlpRevlens/VIDEO/Global.h"

/*
 * The VIDEO_FrameList object uses a linked list for fast insertion
 * and iteration in insertion order. The QHash is used for fast key
 * existance check.
 *
 * The end effect is that this data structure runs in constant time O(1)
 * for the tight-loop operations - insert, index, and can be iterated on
 * in insertion order.
 *
 * Other Options: QHash iteration is arbitrary so cannot be used on its own.
 * QMap is slower than QHash for index at O(log n), although it does have
 * key order iteration. QHash is chosen over QMap for algorithmic complexity -
 * QHash is O(1) for index.
 *
 * http://doc.qt.io/qt-5/containers.html#algorithmic-complexity
 */
 
class REVLENS_VIDEO_API VIDEO_FrameList {
    
public:
    
    typedef QList<qlonglong>::iterator iterator;
    
    VIDEO_FrameList() {}
    
    void push_back(qlonglong frameNum) { 
        _frameList.push_back(frameNum);
        _frameIndex[frameNum] = true;
    }
    
    void push_front(qlonglong frameNum) {
        _frameList.push_front(frameNum);
        _frameIndex[frameNum] = true;
    }
    
    bool contains(qlonglong frameNum) { 
        return _frameIndex.contains(frameNum); 
    }
    
    void clear() {
        _frameList.clear();
        _frameIndex.clear();
    }
    
    
    qlonglong size() { return _frameList.size(); }
    
    qlonglong at(int pos) { return _frameIndex[pos]; }
    
    QList<qlonglong>::iterator begin() { return _frameList.begin(); }
    QList<qlonglong>::iterator end() { return _frameList.end(); }
    
private:
    
    QList<qlonglong> _frameList; // ordered O(1) insertion
    QHash<qlonglong, bool> _frameIndex; // Amortized O(1) lookup
};

class REVLENS_VIDEO_API VIDEO_LookaheadCache : public QObject {
    Q_OBJECT
    
public:

    RLP_DEFINE_LOGGER
    
    
    VIDEO_LookaheadCache();
    
    void setController(DS_ControllerBase* controller)
    {
        _controller = controller;
    }
    
    
    
    bool event(QEvent* event);
    
    bool present(qlonglong frameNum, DS_FrameBufferPtr destFrame);
    bool presentImmediate(qlonglong frameNum,
                          DS_FrameBufferPtr destFrame,
                          bool schedule = true,
                          bool scheduleForceRead = false,
                          bool recenterRequested = false);
    
    
    
public slots:
    
    void recenter(qlonglong frameNum);
    bool isCached(qlonglong frameNum);

    DS_FrameBufferPtr getFrameBuffer(qlonglong frameNum)
    {
        if (_masterIndex.contains(frameNum))
        {
            return _masterIndex[frameNum];
        }

        return nullptr;
    }

    void expireAll();
    
    void setPlaybackMode(DS_PlaybackMode* playbackMode) {
        _playbackMode = playbackMode;
    }
    
    int getBytesRequiredForFrame(qlonglong frameNum);
    
    
    qlonglong maxSize() { return _memLimit; }
    void setMaxSize(qlonglong size); 
    
    int maxBackwardFrameCount() { return _backwardFrameCountMax; }
    void setMaxBackwardFrameCount(int numFrames);

    QList<qlonglong> cachedFrames() { return _masterIndex.keys(); }

signals:
    
    void lookaheadAdd(qlonglong frameNum);
    void lookaheadRelease(qlonglong frameNum);
    void lookaheadClear();
    
    void frameReadEvent(MEDIA_ReadEvent* fre);
    
        
private:
    
    DS_SessionPtr session();
    
    void buildTargetFrameList(qlonglong frameNum, VIDEO_FrameList* targetFrameList);
    
    void buildCacheFrameList(VIDEO_FrameList* targetFrameList,
                             VIDEO_FrameList* cacheFrameList);
    
    void buildAvailableFrameList(VIDEO_FrameList* targetFrameList,
                                 VIDEO_FrameList* availableFrameList);
    
    void releaseFrames(VIDEO_FrameList* availableFrameList);
    
    void scheduleFrames(VIDEO_FrameList* cacheFrameList,
                        VIDEO_FrameList* availableFrameList);
    
private:
    
    qlonglong _memLimit; // memory limit in bytes
    qlonglong _currSize; // current memory usage in bytes
    
    int _backwardFrameCountMax;
    
    VIDEO_FrameList _frameList;
    VIDEO_FrameList _cacheList;
    VIDEO_FrameList _availableFrameList;
    
    QHash<qlonglong, DS_FrameBufferPtr> _masterIndex; // master data index
    
    typedef QHash<qlonglong, DS_FrameBufferPtr>::iterator MasterIndexIterator;
    
    DS_FrameBufferPtr _nullFrame;
    
private:
    
    DS_ControllerBase* _controller;
    DS_PlaybackMode* _playbackMode;
};

#endif
