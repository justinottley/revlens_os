//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//
//

#include "RlpRevlens/VIDEO/LookaheadCache.h"

#include "RlpRevlens/DS/Time.h"
#include "RlpRevlens/MEDIA/Plugin.h"


RLP_SETUP_LOGGER(revlens, VIDEO, LookaheadCache)


#ifdef SCAFFOLD_IOS
static const int LOOKAHEAD_CACHE_SIZE = 420000000;
#else
// static const int LOOKAHEAD_CACHE_SIZE = 20000000;
static const int LOOKAHEAD_CACHE_SIZE = 380000000;
#endif

// _memLimit(320000000),
// _memLimit(750000000), // 500 MB
// _memLimit(450000000),
// _memLimit(350000000),

VIDEO_LookaheadCache::VIDEO_LookaheadCache() :
    _memLimit(LOOKAHEAD_CACHE_SIZE),
    _backwardFrameCountMax(0),
    _currSize(0),
    _nullFrame(new DS_NullFrameBuffer(this))
{
    _frameList.clear();
    _cacheList.clear();
    _availableFrameList.clear();

    _nullFrame->setReady();
}


DS_SessionPtr
VIDEO_LookaheadCache::session()
{
    return _controller->session();
}


void
VIDEO_LookaheadCache::setController(DS_ControllerBase* controller)
{
    // RLP_LOG_DEBUG("")
    _controller = controller;
}


void
VIDEO_LookaheadCache::setMaxSize(qlonglong size)
{
    RLP_LOG_DEBUG(size);

    _memLimit = size;
}


void
VIDEO_LookaheadCache::setMaxBackwardFrameCount(int numFrames)
{
    RLP_LOG_DEBUG(numFrames);

    _backwardFrameCountMax = numFrames;
}


bool
VIDEO_LookaheadCache::event(QEvent* event)
{
    if (event->type() == MEDIA_FrameReadEventType)
    {

        MEDIA_ReadEvent* fre = dynamic_cast<MEDIA_ReadEvent *>(event);

        qlonglong frameNum = fre->getTimeInfo()->videoFrame();

        // RLP_LOG_DEBUG("Got Frame Read:" << frameNum);
        DS_BufferPtr fbuf = fre->getFrameBuffer();

        fbuf->setValid();
        fbuf->setReady();

        if ((_controller->getPlaybackState() == DS_ControllerBase::PLAYSTATE_PAUSED) &&
            (fbuf->getBufferMetadata()->value("video.frame_quality") != (int)DS_FrameBuffer::FRAME_QUALITY_FULL))
        {
            emit frameReadEvent(fre);
        }

        else
        {
            // RLP_LOG_DEBUG(_controller->currentFrameNum() << frameNum)

            if (_controller->currentFrameNum() == frameNum)
            {
                RLP_LOG_DEBUG("Force a display refresh, got current frame:" << frameNum)
                emit frameReadEvent(fre);
            }

            if (_masterIndex.contains(frameNum))
            {
                _masterIndex[frameNum]->setReady();
                emit lookaheadAdd(frameNum);
            }
        }

        return true;
    }

    return QObject::event(event);
}

// -------


bool
VIDEO_LookaheadCache::isCached(qlonglong frameNum)
{
    return _masterIndex.contains(frameNum) && _masterIndex[frameNum]->isReady();
}


int
VIDEO_LookaheadCache::getBytesRequiredForFrame(qlonglong frameNum)
{
    int byteCount = 0;
    DS_NodePtr node = session()->getNodeByFrame(frameNum);

    if (node != nullptr)
    {
        // Getting plugin from node to then pass node in again..
        //
        MEDIA_Plugin* plugin = node->getProperty<MEDIA_Plugin*>("video.plugin");
        byteCount = plugin->getBytesRequiredForFrame(node, frameNum);

        // RLP_LOG_DEBUG("got frame info bytecount: " << frameNum << " " << byteCount);
    }

    return byteCount;
}


void
VIDEO_LookaheadCache::scheduleFrames(VIDEO_FrameList* cacheFrameList,
                                     VIDEO_FrameList* availableFrameList)

{
    qlonglong memRemaining = _memLimit - _currSize;

    VIDEO_FrameList::iterator it;
    for (it = cacheFrameList->begin(); it != cacheFrameList->end(); it++)
    {

        qlonglong frameNum = (*it);

        DS_FrameBufferPtr result = nullptr;

        DS_NodePtr node = session()->getNodeByFrame(frameNum);

        if (node == nullptr)
        {
            RLP_LOG_WARN("no node at frame " << frameNum << " - using null frame buffer");

            _masterIndex[frameNum] = _nullFrame;
            emit lookaheadAdd(frameNum);
            continue;

        }


        // Get node frame rate, build TimeInfo
        float nodeFrameRate = node->getProperty<float>("video.frame_rate");

        // int nodeIndex = node->getProperty<int>("index");

        DS_TimePtr timeInfo = DS_TimePtr(new DS_Time(nodeFrameRate));
        timeInfo->setVideoFrame(frameNum); // session()->getMediaFrameNum(frameNum));
        timeInfo->setVideoFrameOffset(session()->getNodeFrameIndex(node));

        QVariantMap frameInfo = node->getProperty<QVariantMap>("video.frame_info.one");

        // Procure a destination frame buffer for this frame
        int bytesRequiredForFrame = frameInfo.value("byteCount").value<int>();
        if (bytesRequiredForFrame < memRemaining)
        {

            // RLP_LOG_DEBUG("Allocating " << bytesRequiredForFrame << " : " << frameNum);


            // we can allocate a new reference counted frame buffer
            //
            MEDIA_Plugin* plugin = node->getProperty<MEDIA_Plugin*>("video.plugin");
            DS_FrameBufferPtr fbuf = plugin->getOrCreateFrameBuffer(this, frameInfo);
            QVariantMap* fbufMdata = fbuf->getBufferMetadata();

            fbufMdata->insert("video.frame_quality", DS_FrameBuffer::FRAME_QUALITY_FULL);

            QVariant v;
            v.setValue(node);
            fbufMdata->insert("node", v);

            fbuf->setValid();
            fbuf->setReady(false);
            fbuf->reallocate();

            _masterIndex[frameNum] = fbuf;

            memRemaining -= bytesRequiredForFrame;
            _currSize += bytesRequiredForFrame;

            result = fbuf;

        } else
        {

            // RLP_LOG_DEBUG("VIDEO_LookaheadCache::scheduleFrame(): NOT ALLOCATING, LOOKAHEAD FULL!! " << frameNum);
        }

        if (result != nullptr)
        {
            // RLP_LOG_DEBUG(frameNum << "to" << node.get())

            node->scheduleReadTask(
                timeInfo,
                result,
                DS_Node::DATA_VIDEO
            );

        } else
        {
            // RLP_LOG_WARN("Not dispatched:" << frameNum << " " << _currSize << " "<< _memLimit);
        }
    }
}


void
VIDEO_LookaheadCache::releaseFrames(VIDEO_FrameList* availableFrames)
{
    VIDEO_FrameList::iterator it;

    // RLP_LOG_DEBUG("")

    for (it = availableFrames->begin(); it != availableFrames->end(); it++)
    {

        qlonglong frameNum = (*it);

        // RLP_LOG_DEBUG(frameNum)

        DS_NodePtr node = session()->getNodeByFrame(frameNum);
        if (node != nullptr)
        {
            node->scheduleReleaseTask(frameNum);

            DS_FrameBufferPtr fbuf = _masterIndex[frameNum];

            // Store FrameBuffer for reuse rather than create a new
            // framebuffer object
            // better memory allocation management especially for wasm
            //
            int dataBytes = fbuf->getDataBytes(/* auxBuffers= */ true);
            _currSize -= dataBytes;

            DS_FrameBuffer::discardForReuse(fbuf);

            _masterIndex.remove(frameNum);
            emit lookaheadRelease(frameNum);
        }
    }

    // RLP_LOG_DEBUG(std::endl;
}

void
VIDEO_LookaheadCache::buildAvailableFrameList(VIDEO_FrameList* targetFrameList,
                                              VIDEO_FrameList* availableFrameList)
{
    // hmm.. this is gonna build available frames at random....
    MasterIndexIterator it;

    // RLP_LOG_DEBUG(std::endl;
    // RLP_LOG_DEBUG("available frame list: ";

    for (it = _masterIndex.begin(); it != _masterIndex.end(); it++) {

        if (!_masterIndex[it.key()]->isReady())
        {

            // these frames are "in transit" - don't do anything to them
            continue;
        }

        if (!targetFrameList->contains(it.key()))
        {
            availableFrameList->push_back(it.key());

            // RLP_LOG_DEBUG( it.key() << " ";
        }
    }

    // RLP_LOG_DEBUG(std::endl;

}


void
VIDEO_LookaheadCache::buildCacheFrameList(VIDEO_FrameList* targetFrameList,
                                          VIDEO_FrameList* cacheFrameList)
{
    VIDEO_FrameList::iterator it;

    for (it = targetFrameList->begin(); it != targetFrameList->end(); it++)
    {

        if (!_masterIndex.contains((*it)))
        {
            cacheFrameList->push_back(*it);

            // RLP_LOG_DEBUG((*it) << " ";
        }
    }
}


void
VIDEO_LookaheadCache::buildTargetFrameList(qlonglong frameNum,
                                           VIDEO_FrameList* targetFrameList)
{
    qlonglong totalFrames = qlonglong(rint((float)session()->frameCount() / (float)_playbackMode->getFrameIncrement()));

    if (totalFrames == 0)
    {
        RLP_LOG_WARN("TOTAL FRAMES 0, ABORTING");
        return;
    }

    qlonglong currBackwardFrames = 0;
    bool marchForward = true;

    int currByteCount = getBytesRequiredForFrame(frameNum); // 0;
    qlonglong numFrames = 0;

    qlonglong nextFrame = frameNum;

    qlonglong forwardEdgeFrame = frameNum;
    qlonglong backwardEdgeFrame = frameNum;

    // RLP_LOG_DEBUG("totalFrames:" << totalFrames);

    DS_PlaybackMode::DirectionMode nextDirection = _playbackMode->getDirection();

    while ((currByteCount < _memLimit) && (numFrames < totalFrames))
    {

        targetFrameList->push_back(nextFrame);

        // std::cout << nextFrame << " ";

        currByteCount += getBytesRequiredForFrame(nextFrame);

        if (currBackwardFrames < _backwardFrameCountMax)
        {
            marchForward = !marchForward;
        } else
        {
            marchForward = true;
        }

        if (marchForward)
        {
            std::pair<qlonglong, DS_PlaybackMode::DirectionMode> nextFrameNumInfo = _playbackMode->getNextFrameNum(
                forwardEdgeFrame,
                nextDirection,
                DS_PlaybackMode::INC
            );
            forwardEdgeFrame = nextFrameNumInfo.first;
            nextDirection = nextFrameNumInfo.second;

            nextFrame = forwardEdgeFrame;

        } else
        {
            std::pair<qlonglong, DS_PlaybackMode::DirectionMode> nextFrameNumInfo = _playbackMode->getNextFrameNum(
                backwardEdgeFrame,
                nextDirection,
                DS_PlaybackMode::DEC
            );

            nextFrame = backwardEdgeFrame;
            currBackwardFrames++;
        }

        numFrames++;
    }

    // std::cout << "frame " << frameNum << " bytecount: " << currByteCount << std::endl;

}


void
VIDEO_LookaheadCache::expireAll()
{
    RLP_LOG_DEBUG("");

    MasterIndexIterator it;

    // RLP_LOG_DEBUG(std::endl;
    // RLP_LOG_DEBUG("available frame list: ";

    for (it = _masterIndex.begin(); it != _masterIndex.end(); ++it)
    {
        it.value()->setInvalid();
    }

    _masterIndex.clear();
    _currSize = 0;

    RLP_LOG_DEBUG("Done");

    emit lookaheadClear();
}


void
VIDEO_LookaheadCache::cancelRequestedFrames()
{
    // RLP_LOG_DEBUG("")

    QSet<DS_NodePtr> seenNodes;
    MasterIndexIterator it;

    VIDEO_FrameList releaseList;

    for (it = _masterIndex.begin(); it != _masterIndex.end(); ++it)
    {
        DS_NodePtr node = it.value()->getBufferMetadata()->value("node").value<DS_NodePtr>();
        if (!seenNodes.contains(node))
        {
            if (node != nullptr)
            {
                MEDIA_Plugin* plugin = node->getProperty<MEDIA_Plugin*>("video.plugin");
                plugin->getReadThreadPool(DS_Node::DATA_VIDEO)->clear();
            } else
            {
                RLP_LOG_ERROR("Invalid node found at" << it.key())
            }
            
            seenNodes.insert(node);
        }

        if (!it.value()->isReady())
        {
            releaseList.push_back(it.key());
        }
    }

    releaseFrames(&releaseList);
}


void
VIDEO_LookaheadCache::recenter(qlonglong frameNum)
{
    // RLP_LOG_DEBUG(frameNum << _currSize);

    VIDEO_FrameList targetFrameList;
    VIDEO_FrameList cacheFrameList;
    VIDEO_FrameList availableFrameList;

    // build a list of all the frames that should be in the lookahead
    // centered at frameNum
    buildTargetFrameList(frameNum, &targetFrameList);

    // of all the frames we need, build a list of frames that require caching.
    // Ideally most of the frames will already be in the lookahead
    buildCacheFrameList(&targetFrameList, &cacheFrameList);

    // build a list of expired frames inside the lookahead that we no longer
    // care about
    buildAvailableFrameList(&targetFrameList, &availableFrameList);

    // release frames that have expired
    releaseFrames(&availableFrameList);

    // dispatch the new frames we need for reading
    scheduleFrames(&cacheFrameList, &availableFrameList);
}


bool
VIDEO_LookaheadCache::present(qlonglong frameNum, DS_FrameBufferPtr destFrame)
{
    // RLP_LOG_DEBUG(frameNum);

    bool result = true;

    recenter(frameNum);

    if (_masterIndex.contains(frameNum))
    {
        if (_masterIndex[frameNum]->isReady())
        {
             _masterIndex[frameNum]->copyTo(destFrame);

        } else
        {
            RLP_LOG_ERROR("Master index frame NOT READY:" << frameNum);

            DS_FrameBufferPtr fbuf = _masterIndex[frameNum];
            DS_NodePtr node = fbuf->getBufferMetadata()->value("node").value<DS_NodePtr>();

            DS_TimePtr timeInfo = node->makeTime(frameNum);

            fbuf->getBufferMetadata()->insert("video.frame_quality", DS_FrameBuffer::FRAME_QUALITY_FAST);
            node->readVideo(timeInfo, fbuf, /*postUpdate=*/false);

            // RLP_LOG_DEBUG("blocking immediate read:" << timeInfo->videoFrame() << "media:" << timeInfo->mediaVideoFrame())
            fbuf->copyTo(destFrame);
        }

    } else
    {

        RLP_LOG_ERROR("Master index doesn't have this frame: " << frameNum);

        // TODO: If frame isn't there, stop to buffer
        // result = false;
    }

    return result;
}


bool
VIDEO_LookaheadCache::presentImmediate(qlonglong frameNum,
                                       DS_FrameBufferPtr destFrame,
                                       bool schedule,
                                       bool recenterRequested)
{
    // RLP_LOG_DEBUG(frameNum << schedule);

    bool result = true;

    if (_masterIndex.contains(frameNum) && _masterIndex[frameNum]->isReady())
    {

        // RLP_LOG_DEBUG(frameNum << ":OK");

        _masterIndex[frameNum]->copyTo(destFrame);

    } else
    {

        // RLP_LOG_DEBUG(frameNum << " requires reading")

        result = false;

        DS_NodePtr node = session()->getNodeByFrame(frameNum);
        if (node == nullptr)
        {
            return result;
        }

        // RLP_LOG_DEBUG("attempting to get frame info")

        QVariantMap frameInfo = node->getProperty<QVariantList>("video.frame_info").at(0).toMap();
        DS_TimePtr timeInfo = node->makeTime(frameNum);

        // RLP_LOG_DEBUG("got frame info")


        MEDIA_Plugin* plugin = node->getProperty<MEDIA_Plugin*>("video.plugin");
        DS_FrameBufferPtr fbuf = plugin->getOrCreateFrameBuffer(this, frameInfo);


        fbuf->getBufferMetadata()->insert("video.frame_quality", DS_FrameBuffer::FRAME_QUALITY_FAST);
        fbuf->setValid();
        fbuf->reallocate();

        #ifdef SCAFFOLD_IOS
        schedule = false;
        #endif

        if (schedule)
        {

            // RLP_LOG_DEBUG("scheduling " << frameNum << "to" << node.get())

            // schedule this frame for reading asynchronously
            node->scheduleReadTask(
                timeInfo,
                fbuf,
                DS_Node::DATA_VIDEO,
                true
            );


        } else
        {

            RLP_LOG_WARN("blocking immediate read - " << frameNum);

            // blocking immediate read
            node->readVideo(timeInfo, fbuf, false);
            fbuf->copyTo(destFrame);

            result = true;
        }
    }

    if (recenterRequested)
    {
        cancelRequestedFrames();
        recenter(frameNum);
    }

    return result;
}
