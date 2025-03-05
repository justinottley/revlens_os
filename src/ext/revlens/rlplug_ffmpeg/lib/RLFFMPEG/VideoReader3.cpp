//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include "RlpExtrevlens/RLFFMPEG/VideoReader3.h"
#include "RlpExtrevlens/RLFFMPEG/Plugin.h"

#include "RlpRevlens/MEDIA/Plugin.h"
#include "RlpRevlens/DS/AudioBuffer.h"




RLP_SETUP_EXT_LOGGER(RLFFMPEG, VideoReader)

RLFFMPEG_VideoReader::RLFFMPEG_VideoReader(QVariantMap* properties) :
    DS_Node("RLFFMPEG_VideoReader", properties),
    _AVMEDIA_TYPE(AVMEDIA_TYPE_VIDEO),
    _fmtCtx(NULL),
    _streamDecCtx(NULL),
    _readerType(QString("video")),
    _ptsOffset(0),
    _swsCtx(NULL),
    _lastVideoFrame(-1),
    _flushMode(false),
    _thumbCache(nullptr),
    _thumbCacheState(RLFFMPEG_VideoReader::THUMB_CACHE_UNINITIALIZED),
    _thumbCacheTask(nullptr)
{
    RLP_LOG_DEBUG("");

    QString srcPathKey("media.");
    srcPathKey += _readerType;

    _srcFilePath = properties->value(srcPathKey).value<QString>();
    _frameRate = properties->value("video.frame_rate").value<float>();

    _lastReadResult.av_frame = NULL;

    _properties->insert("video.src_file_path", _srcFilePath);

    init();
}


void
RLFFMPEG_VideoReader::init()
{
    RLP_LOG_DEBUG("")

    openMedia();
    initializeScalars();
    resetAll();
}


void
RLFFMPEG_VideoReader::closeMedia()
{
    // RLP_LOG_DEBUG("");

    _flushMode = false;

    avcodec_flush_buffers(_streamDecCtx);

    avcodec_free_context(&_streamDecCtx);
    avcodec_close(_streamDecCtx);

    avformat_close_input(&_fmtCtx);

    sws_freeContext(_swsCtx);
}


void
RLFFMPEG_VideoReader::resetAll()
{
    // RLP_LOG_DEBUG("");

    closeMedia();
    openMedia();
}


bool
RLFFMPEG_VideoReader::openMedia()
{
    // RLP_LOG_DEBUG("")

    // Create video decode context
    if (initializeCodecContext() >= 0)
    {
        initializeDestFormatContext();
    }


    return true;
}




void
RLFFMPEG_VideoReader::initializeScalars()
{
    RLP_LOG_DEBUG("")


    uint64_t currPts;
    long ptsSum = 0;
    long sampleSum = 0;

    uint64_t lastPts = 0;

    for (int f=0; f!=13; ++f) {

        RLFFMPEG_PacketReadResult p_ret = readNextPacket();

        /*
        RLP_LOG_DEBUG("RLFFMPEG_ReaderBase::initializeScalars() : "
                  << "[ " << _readerType.toStdString().c_str() << " ] "
                  << "f: " << f
                  << " pts: " << p_ret.pts_result
                  << " dts: " << p_ret.dts_result
                  << " nb_samples: " << p_ret.av_frame->nb_samples
                  );
        */

        currPts = p_ret.pts_result;

        // RLP_LOG_DEBUG("reading frame " << f << " : " << currPts );

        if (f == 1) {
            _ptsOffset = currPts;
            // RLP_LOG_DEBUG("dts offset: " << _ptsOffset );
        }

        if (f < 3) {
            lastPts = currPts;
            continue;
        }

        sampleSum += p_ret.av_frame->nb_samples; // p_ret.nb_samples;
        ptsSum += (currPts - lastPts);
        lastPts = currPts;

        av_frame_unref(p_ret.av_frame);
        av_frame_free(&p_ret.av_frame);

    }

    _ptsIncrement = round(float(ptsSum) / 10.0);



    if (!_properties->contains("video.frame_count")) {
        RLP_LOG_WARN("NO FRAMECOUNT");
    }

    _ptsMax = getDts(_properties->value("video.frame_count").value<float>());

    QVariant ptsi;
    ptsi.setValue(_ptsIncrement);

    QVariant ptso;
    ptso.setValue(_ptsOffset);

    _properties->insert(_readerType + ".pts_increment", ptsi);
    _properties->insert(_readerType + ".pts_offset", ptso);

    RLP_LOG_DEBUG("RLFFMPEG_VideoReader::initializeScalars() : "
              << "[ " << _readerType << " ]"
              << " pts sum: " << ptsSum
              << " pts offset: " << _ptsOffset
              << " pts increment: " << _ptsIncrement
              << " pts max: " << _ptsMax
              );

}


int
RLFFMPEG_VideoReader::initializeCodecContext()
{
    // RLP_LOG_DEBUG("");

    int ret;
    AVStream *st;
    // AVCodecContext *dec_ctx = NULL;
    const AVCodec *dec = NULL;
    AVDictionary *opts = NULL;

    int openResult = 0;

    QByteArray srcfileba = _srcFilePath.toLocal8Bit();
    const char* srcPath = srcfileba.data();

    // RLP_LOG_DEBUG("Opening file..")

    // open input file, and allocate format context
    if (avformat_open_input(&_fmtCtx, srcPath, NULL, NULL) < 0)
    {
        RLP_LOG_ERROR("Could not open source file: " << _srcFilePath)
        openResult = -1;
    }

    // RLP_LOG_DEBUG("Retreiving stream information..")

    // retrieve stream information
    if (avformat_find_stream_info(_fmtCtx, NULL) < 0)
    {
        RLP_LOG_ERROR("Could not find stream information");
        openResult = -1;
    }

    // RLP_LOG_DEBUG("Finding best stream..")

    ret = av_find_best_stream(_fmtCtx, _AVMEDIA_TYPE, -1, -1, NULL, 0);
    if (ret < 0)
    {

        QString msg = "Could not find ";
        msg += av_get_media_type_string(_AVMEDIA_TYPE);
        msg += " stream in input file ";
        msg += _srcFilePath;

        RLP_LOG_ERROR(msg);

        return ret;

    } else
    {

        int stream_index = ret;

        st = _fmtCtx->streams[stream_index];

        /* find decoder for the stream */
        dec = avcodec_find_decoder(st->codecpar->codec_id);
        if (!dec)
        {
            fprintf(stderr, "Failed to find %s codec\n",
                    av_get_media_type_string(_AVMEDIA_TYPE));
            return AVERROR(EINVAL);
        }

        /* Allocate a codec context for the decoder */
        _streamDecCtx = avcodec_alloc_context3(dec);
        if (!_streamDecCtx)
        {
            fprintf(stderr, "Failed to allocate the %s codec context\n",
                    av_get_media_type_string(_AVMEDIA_TYPE));
            return AVERROR(ENOMEM);
        }

        /* Copy codec parameters from input stream to output codec context */
        if ((ret = avcodec_parameters_to_context(_streamDecCtx, st->codecpar)) < 0)
        {
            fprintf(stderr, "Failed to copy %s codec parameters to decoder context\n",
                    av_get_media_type_string(_AVMEDIA_TYPE));
            return ret;
        }

        /* Init the decoders, with or without reference counting */
        // av_dict_set(&opts, "refcounted_frames", "1", 0);

        if ((ret = avcodec_open2(_streamDecCtx, dec, &opts)) < 0)
        {
            fprintf(stderr, "Failed to open %s codec\n",
                    av_get_media_type_string(_AVMEDIA_TYPE));
            return ret;
        }

        _streamIdx = stream_index;

        // av_dump_format(_fmtCtx, 0, _srcFilePath.toStdString().c_str(), 0);

        // RLP_LOG_DEBUG("Codec info: " << _streamDecCtx->width << " " << _streamDecCtx->height );
    }

    return 0;

}



void
RLFFMPEG_VideoReader::initializeDestFormatContext()
{
    // RLP_LOG_DEBUG("");

    int ret;


    // create scaling context
    _swsCtx = sws_getContext(_streamDecCtx->width,
                             _streamDecCtx->height,
                             _streamDecCtx->pix_fmt,
                             _streamDecCtx->width,
                             _streamDecCtx->height,
                             AV_PIX_FMT_RGB32,
                             SWS_BICUBIC, NULL, NULL, NULL);
    if (!_swsCtx)
    {
        fprintf(stderr,
                "Impossible to create scale context for the conversion "
                "fmt:%s s:%dx%d -> fmt:%s s:%dx%d\n",
                av_get_pix_fmt_name(_streamDecCtx->pix_fmt),
                _streamDecCtx->width,
                _streamDecCtx->height,
                av_get_pix_fmt_name(AV_PIX_FMT_RGB32),
                _streamDecCtx->width,
                _streamDecCtx->height);

        ret = AVERROR(EINVAL);
        exit(1);
    }

    int result = sws_setColorspaceDetails(_swsCtx,
                                          sws_getCoefficients(SWS_CS_ITU709), // inv_table
                                          0, // srcRange -flag indicating the white-black range of the input (1=jpeg / 0=mpeg) 0 = 16..235, 1 = 0..255
                                          sws_getCoefficients(SWS_CS_DEFAULT), // table
                                          1, // dstRange - 0 = 16..235, 1 = 0..255
                                          0, // brightness fixed point, with 0 meaning no change,
                                          1 << 16, // contrast   fixed point, with 1<<16 meaning no change,
                                          1 << 16); // saturation fixed point, with 1<<16 meaning no change);
    if (result == -1)
    {
        RLP_LOG_ERROR("ERROR SETTING UP COLOURSPACE DETAILS FOR SWS_CTX" );
    }

    // Allocate rescale buffer
    //
    // setup _rgbLineSize - I don't think this is how this is supposed to be done
    // but it works - may have to be revisited in future for setting up
    // dest linesize array for rescale context
    //
    uint8_t* tmpRgbData[4];
    if ((ret = av_image_alloc(&tmpRgbData[0],
                              _rgbLineSize,
                              _streamDecCtx->width,
                              _streamDecCtx->height,
                              AV_PIX_FMT_RGB32,
                              1)) < 0) {
        fprintf(stderr, "Could not setup destination line size\n");
        exit(1);
    }

    av_free(tmpRgbData[0]);
}


uint64_t
RLFFMPEG_VideoReader::getDts(long frameNum)
{
    return 0;
}



RLFFMPEG_PacketReadResult
RLFFMPEG_VideoReader::seekToFrame(qlonglong frameNum)
{

    if (frameNum == 0) {

        // Close and reopen if we are rereading starting at the
        // beginning (frame 0) - others have found this to be more
        // reliable than trying to seek
        //
        resetAll();

        // p_ret = readNextPacket();
        return readNextPacket();
    }


    double preSeconds = (double(frameNum) / _properties->value("video.frame_rate").value<double>()); //  * 0.995;

    int64_t timestamp = preSeconds / av_q2d(_fmtCtx->streams[_streamIdx]->time_base);


    // RLP_LOG_DEBUG(" SEEK: " << frameNum << " SECONDS: " << preSeconds << " TIMESTAMP: " << timestamp );
    // int flags = AVSEEK_FLAG_BACKWARD;
    int flags = 0;


    avcodec_flush_buffers(_streamDecCtx);

    int ret = avformat_seek_file(_fmtCtx, _streamIdx, 0, timestamp, timestamp, flags);
    if (ret < 0)
    {
        RLP_LOG_ERROR("SEEK FAILED!" );
        // ?? return ??
    }

    // AVFrame* frame = av_frame_alloc();
    RLFFMPEG_PacketReadResult p_ret = readNextPacket();


    // RLP_LOG_DEBUG("[ " << _readerType << " ] "
    //           << "seek ret: " << ret
    //           << " pts: " << p_ret.pts_result
    //           << " dts: " << p_ret.dts_result
    //           );

    if (p_ret.pts_result == 0)
    {
        resetAll();
        p_ret = readNextPacket();
    }


    if (p_ret.pts_result > timestamp)
    {
        RLP_LOG_DEBUG("seek returned pts greater than timestamp, resetting to start");

        // TODO FIXME: seek faster by going back to the next previous keyframe
        // instead of the start - this could potentially be unsuitable / infeasible for large files
        //
        resetAll();
        p_ret = readNextPacket();
    }


    uint64_t currDts = p_ret.pts_result;

    while ((currDts + _ptsIncrement - 1) < timestamp) {

        // RLP_LOG_DEBUG("[ " << _readerType.toStdString().c_str() << " ]" << " Marching forward - " << currDts );

        av_frame_unref(p_ret.av_frame);
        av_frame_free(&p_ret.av_frame);

        p_ret = readNextPacket();

        if (p_ret.return_status == DS_Buffer::READ_EOF) {
            RLP_LOG_ERROR("SEEK FAILED - END OF FILE");
            return p_ret;
        }

        currDts = p_ret.pts_result;

    }


    // RLP_LOG_DEBUG("[ " << _readerType << " ]"
    //           << " Ending on " << p_ret.pts_result );

    return p_ret;
}



bool
RLFFMPEG_VideoReader::getFrame(AVPacket* pkt, AVFrame* frame)
{
    int ret_send_pkt = avcodec_send_packet(_streamDecCtx, pkt);
    if(ret_send_pkt < 0)
    {
        // fprintf(stderr, "send packet failed %i - %s\n", ret_send_pkt, av_err2str(ret_send_pkt));

        RLP_LOG_WARN("Send packet failed")

        return false;
    }


    int ret_receive_frame = avcodec_receive_frame(_streamDecCtx, frame);
    if (ret_receive_frame < 0)
    {

        // RLP_LOG_DEBUG("avcodec_receive_frame(): " << av_err2str(ret_receive_frame));

        if (ret_receive_frame == -11)
        {
            // RLP_LOG_DEBUG("avcodec_receive_frame(): " << av_err2str(ret_receive_frame));

        } else
        {
            // fprintf(stderr, "receive frame failed %i - %s\n", ret_receive_frame, av_err2str(ret_receive_frame));

            RLP_LOG_VERBOSE("receive frame failed")

        }

        // RLP_LOG_DEBUG("AVFrame pts: " << frame->pts << " AVPacket pts: " << frame->pkt_pts);

        return false;
    }

    // RLP_LOG_DEBUG("AVFrame pts: " << frame->pkt_pts << " dts: " << frame->pkt_dts  << " keyframe: " << frame->key_frame);;
    // RLP_LOG_DEBUG("readNextPacket(): pts: " << p_ret.pts_result << " dts: " << p_ret.dts_result << " keyframe: " << p_ret.av_frame->key_frame << " return result: " << p_ret.return_status );

    return true;

}


RLFFMPEG_PacketReadResult
RLFFMPEG_VideoReader::readNextPacket()
{
    // RLP_LOG_DEBUG("");


    RLFFMPEG_PacketReadResult p_ret;
    p_ret.dts_result = 0;
    p_ret.pts_result = 0;
    p_ret.return_status = DS_Buffer::READ_UNKNOWN;
    p_ret.nb_samples = 0;
    p_ret.av_frame = av_frame_alloc();

    if (_flushMode) {

        // See https://ffmpeg.org/doxygen/3.1/group__lavc__encdec.html

        int ret_receive_frame = avcodec_receive_frame(_streamDecCtx, p_ret.av_frame);
        if (ret_receive_frame != 0) {
            // fprintf(stderr, "flush mode - receive frame failed %i - %s\n", ret_receive_frame, av_err2str(ret_receive_frame));

            RLP_LOG_DEBUG("flush mode - receive frame failed")

            p_ret.return_status = DS_Buffer::READ_EOF;
        }

        return p_ret;
    }



    int got_frame = false;
    while (!got_frame) {

        // RLP_LOG_DEBUG("readNextPacket() looping in while" );

        AVPacket my_pkt;
        int arf_ret = av_read_frame(_fmtCtx, &my_pkt);

        // RLP_LOG_DEBUG("av_read_frame(): " << arf_ret << " streamidx: " << my_pkt.stream_index << "dts: " << my_pkt.dts << "pts: " << my_pkt.pts);

        if (arf_ret >= 0) {

            if (my_pkt.stream_index == _streamIdx) {


                if(!getFrame(&my_pkt, p_ret.av_frame)) {

                    // RLP_LOG_DEBUG("LOOPING IN GET FRAME" );

                    p_ret.return_status = DS_Buffer::READ_ERROR;

                    // RLP_LOG_ERROR("GET FRAME FAILED" );

                    av_packet_unref(&my_pkt);

                    // exit(1);
                    continue;
                }

                got_frame = true;

            }

        } else if (arf_ret == -541478725) {


            // End of file - enter flush mode to drain the codec and read any last frames
            //

            // RLP_LOG_WARN("end of file detected - entering flush mode");

            _flushMode = true;

            // enter draining mode by sending NULL packet
            //
            int ret_send_pkt = avcodec_send_packet(_streamDecCtx, NULL);
            if (ret_send_pkt == 0) {

                // RLP_LOG_DEBUG("flush mode succesfull");

                int ret_receive_frame = avcodec_receive_frame(_streamDecCtx, p_ret.av_frame);
                if (ret_receive_frame != 0) {
                    // fprintf(stderr, "flush mode - receive frame failed %i - %s\n", ret_receive_frame, av_err2str(ret_receive_frame));
                    RLP_LOG_DEBUG("flush mode - receieve frame failed")

                }

            } else {
                p_ret.return_status = DS_Buffer::READ_EOF;

            }


            got_frame = true;


            p_ret.return_status = DS_Buffer::READ_EOF;

        } else {

            // fprintf(stderr, "Error decoding frame - %i - %s\n", arf_ret, av_err2str(arf_ret));

            // fprintf(stderr, "ERROR DECODING FRAME - %i %s\n", arf_ret, av_err2str(arf_ret));
            RLP_LOG_ERROR("ERROR DECODING FRAME")

            got_frame = true;

            p_ret.return_status = DS_Buffer::READ_EOF;

        }

        av_packet_unref(&my_pkt);

    }

    //
    // Set pts_result and dts_result after avcodec_send_packeet / avcodec_receieve_frame pair
    //
    p_ret.pts_result = p_ret.av_frame->pts;
    p_ret.dts_result = p_ret.av_frame->pkt_dts;


    // RLP_LOG_DEBUG("pts: " << p_ret.pts_result << " dts: " << p_ret.dts_result << " keyframe: " << p_ret.av_frame->key_frame << " return result: " << p_ret.return_status );

    return p_ret;
}


void
RLFFMPEG_VideoReader::readVideo(DS_TimePtr timeInfo, DS_BufferPtr destBuffer, bool postUpdate)
{

    if ((destBuffer == nullptr) ||
        (!destBuffer->isValid()))
    {
        RLP_LOG_ERROR("invalid dest buffer, abort read")
        return;
    }

    /*
    RLP_LOG_DEBUG("RLFFMPEG_VideoReader::3::readVideo() : "
              << _srcFilePath.toStdString().c_str() << " "
              << timeInfo->mediaVideoFrame() << " "
              << "offset: " << timeInfo->videoFrameOffset()
              // << timeInfo->playlistVideoFrame() << " "
              // << " last pos: " << _lastVideoFrame
              );
    */

    RLFFMPEG_VideoReader::ThumbCacheState tcs = thumbCacheState();

    // RLP_LOG_DEBUG("Thumb cache state:" << (int)tcs)

    #ifdef SCAFFOLD_IOS
    if (tcs == RLFFMPEG_VideoReader::THUMB_CACHE_UNINITIALIZED)
    {
        scheduleThumbnailGen();
    }
    #endif

    _lock.lock();


    if ((destBuffer->getBufferMetadata()->contains("video.display_immediate")) &&
        (destBuffer->getBufferMetadata()->value("video.display_immediate") != (int)DS_FrameBuffer::FRAME_QUALITY_FULL) &&
        (tcs == RLFFMPEG_VideoReader::THUMB_CACHE_READY))
    {
        qlonglong frameNum = timeInfo->mediaVideoFrame();

        ThumbCacheMap* tcm = _thumbCache; // .localData();
        if (tcm == nullptr)
        {
            RLP_LOG_ERROR("no thumb cache")
        }

        if ((tcm != nullptr) && (tcm->contains(frameNum)))
        {
            QImage i = tcm->value(frameNum);

            // RLP_LOG_DEBUG("Found thumb for frame:" << frameNum << i.width() << "x" << i.height())

            QVariantMap frameInfo;
            frameInfo.insert("width", i.width());
            frameInfo.insert("height", i.height());
            frameInfo.insert("pixelSize", 1);
            frameInfo.insert("channelCount", 4);
            frameInfo.insert("byteCount", i.sizeInBytes());
            frameInfo.insert("hasAlpha", i.hasAlphaChannel());

            MEDIA_Plugin* plugin = getProperty<MEDIA_Plugin*>("video.plugin");
            DS_FrameBufferPtr p = plugin->getOrCreateFrameBuffer(this, frameInfo);

            p->reallocate();
            p->setColourSpace(DS_FrameBuffer::COLSPACE_NONLINEAR);
            // p->setChannelOrder(DS_FrameBuffer::CHANNEL_ORDER_BGRA);
            p->setGLTextureFormat(GL_RGBA);
            p->setGLTextureInternalFormat(GL_RGBA);

            memcpy(p->data(), i.bits(), p->getDataBytes());

            destBuffer->appendAuxBuffer(p);

            // RLP_LOG_DEBUG("dest frame aux buffer list size:" << destFrame->numAuxBuffers(false));

        } else
        {
            RLP_LOG_WARN("frame not found in thumb cache:" << frameNum)
        }
    } else
    {

    RLFFMPEG_PacketReadResult p_ret;

    if ((_lastVideoFrame >= 0) && (timeInfo->mediaVideoFrame() == _lastVideoFrame)) {

        // RLP_LOG_DEBUG("RETURNING LAST READ RESULT" );
        p_ret = _lastReadResult;
    }


    else {

        // something other than the previous frame
        //
        if (_lastReadResult.av_frame != NULL) {

            av_frame_unref(_lastReadResult.av_frame);
            av_frame_free(&_lastReadResult.av_frame);
        }


        if (timeInfo->mediaVideoFrame() < _lastVideoFrame) {

           p_ret = seekToFrame(timeInfo->mediaVideoFrame());

        } else if (std::abs(qlonglong(timeInfo->mediaVideoFrame() - _lastVideoFrame)) > 1) {

            // RLP_LOG_DEBUG("seeking to " << timeInfo->mediaVideoFrame() );
            p_ret = seekToFrame(timeInfo->mediaVideoFrame());

        } else {

            // very next frame
            //
            p_ret = readNextPacket();

        }

    }


    _lastVideoFrame = timeInfo->mediaVideoFrame();
    _lastReadResult = p_ret;


    // convert to destination format
    //
    uint8_t* destBufferData = (uint8_t*)destBuffer->data();
    sws_scale(_swsCtx,
              (const uint8_t **)(p_ret.av_frame->data),
              p_ret.av_frame->linesize,
              0,
              _streamDecCtx->height,
              &destBufferData,
              _rgbLineSize);

    // We are saving the last read result incase the same frame is requested
    // agian, so the av_frame is not freed here
    //
    // av_frame_unref(p_ret.av_frame);
    // av_frame_free(&p_ret.av_frame);


    destBuffer->getBufferMetadata()->insert("video.readResult", p_ret.return_status);
    // Set channel order to BGRA
    //
    dynamic_cast<DS_FrameBuffer*>(destBuffer.get())->setChannelOrder(
        DS_FrameBuffer::CHANNEL_ORDER_BGRA);

    }







    _lock.unlock();



    if (postUpdate) {

        // RLP_LOG_DEBUG("POST UPDATE "  << timeInfo->mediaVideoFrame() << destBuffer->getOwner())

        QApplication::postEvent(destBuffer->getOwner(),
                                new MEDIA_ReadEvent(timeInfo,
                                                    getProperty<int>("index"),
                                                    destBuffer,
                                                    MEDIA_FrameReadEventType));
    }
}


void
RLFFMPEG_VideoReader::scheduleReadTask(
    DS_TimePtr timeInfo,
    DS_BufferPtr destBuffer,
    DS_Node::NodeDataType dataType,
    bool optional)
{
    QRunnable* task = new RLFFMPEG_VideoReadTask(this, timeInfo, destBuffer);
    QThreadPool* tpool = _properties->value("video.plugin").
                                      value<RLFFMPEG_Plugin *>()->
                                      getReadThreadPool(dataType);

    // STARTUP HACK: add an exception for frame 0 as a fix for getting the
    // display to show picture at startup.. not a great solution, especially
    // if the user requests a start frame other than 0 for startup...
    //

    if ((optional) && (timeInfo->mediaVideoFrame() != 0))
    {
        tpool->tryStart(task);
    } else
    {
        tpool->start(task);
    }
}


void
RLFFMPEG_VideoReader::setThumbCacheState(RLFFMPEG_VideoReader::ThumbCacheState tcstate)
{
    RLP_LOG_DEBUG("")

    QMutexLocker l(&_lock);
    _thumbCacheState = tcstate;
}


RLFFMPEG_VideoReader::ThumbCacheState
RLFFMPEG_VideoReader::thumbCacheState()
{
    // RLP_LOG_DEBUG("")

    QMutexLocker l(&_lock);

    return _thumbCacheState;
}


void
RLFFMPEG_VideoReader::scheduleThumbnailGen()
{
    RLP_LOG_DEBUG("")

    if (_thumbCache != nullptr)
    {
        RLP_LOG_ERROR("Skipping, thumbnail cache exists")
        return;
    }

    setThumbCacheState(RLFFMPEG_VideoReader::THUMB_CACHE_RUNNING);

    _thumbCache = new ThumbCacheMap();
    _thumbCacheTask = new RLFFMPEG_ThumbnailCacheTask(this);
    _thumbCacheTask->setAutoDelete(false);

    _thpool.setMaxThreadCount(1);

    _thpool.start(_thumbCacheTask);
    _ttimer.callOnTimeout(this, &RLFFMPEG_VideoReader::continueThumbnailGen);
    _ttimer.setInterval(100);
    _ttimer.start();

}


void
RLFFMPEG_VideoReader::continueThumbnailGen()
{
    RLFFMPEG_VideoReader::ThumbCacheState tcs = thumbCacheState();

    if (tcs == RLFFMPEG_VideoReader::THUMB_CACHE_RUNNING)
    {
        _thpool.tryStart(_thumbCacheTask);
    } else if (tcs == RLFFMPEG_VideoReader::THUMB_CACHE_READY)
    {
        _ttimer.stop();
    }

    DS_Graph* graph = getProperty<DS_Graph*>("graph");
    qlonglong numFrames = getProperty<qlonglong>("media.frame_count");

    QVariantMap data;
    data.insert("data_type", "thumbnail_cache");
    data.insert("thumbnail_cache.count", _thumbCacheTask->fnum());
    data.insert("thumbnail_cache.total", numFrames);
    graph->emitDataReady(data);
}