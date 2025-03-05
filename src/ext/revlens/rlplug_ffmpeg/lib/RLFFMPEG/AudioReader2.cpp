//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//


#include "RlpExtrevlens/RLFFMPEG/AudioReader2.h"
#include "RlpExtrevlens/RLFFMPEG/Plugin.h"

#include "RlpCore/UTIL/AppGlobals.h"

#include "RlpRevlens/AUDIO/AudioTime.h"
#include "RlpRevlens/MEDIA/Plugin.h"


RLP_SETUP_LOGGER(ext, RLFFMPEG, AudioReader)

RLFFMPEG_AudioReader::RLFFMPEG_AudioReader(QVariantMap* properties) :
    DS_Node("RLFFMPEG_AudioReader", properties),
    _AVMEDIA_TYPE(AVMEDIA_TYPE_AUDIO),
    _fmtCtx(NULL),
    _streamDecCtx(NULL),
    _frame(NULL),
    _readerType(QString("audio")),
    _ptsOffset(0),
    _sampleIncrement(0),
    _swrCtx(NULL),
    _timeMultiplier(1),
    _audioStartVideoFrame(0)
{
    QVariantMap* ag = UTIL_AppGlobals::instance()->globalsPtr();

    _outputBitDepth = ag->value("audio.output.bitdepth").toInt();
    _outputChannelCount = ag->value("audio.output.channels").toInt();
    _outputSampleRate = ag->value("audio.output.sampleRate").toInt();

    _emptyAudioBuffer = DS_AudioBufferPtr(
        new DS_AudioBuffer(this, _outputBitDepth, _outputChannelCount, _outputSampleRate));

    _emptyAudioBuffer->reallocate();

    std::memset(_emptyAudioBuffer->data(), 0, _emptyAudioBuffer->getDataBytes());

    QString srcPathKey("media.");
    srcPathKey += _readerType;

    _srcFilePath = properties->value(srcPathKey).toString();

    openMedia();
    initializeScalars();
    resetAll();
}


void
RLFFMPEG_AudioReader::closeMedia()
{
    // RLP_LOG_DEBUG("");

    avcodec_flush_buffers(_streamDecCtx);

    avcodec_free_context(&_streamDecCtx);
    avcodec_close(_streamDecCtx);

    avformat_close_input(&_fmtCtx);

    swr_free(&_swrCtx);
}


void
RLFFMPEG_AudioReader::resetAll()
{
    // RLP_LOG_DEBUG("");

    closeMedia();
    openMedia();
}


bool
RLFFMPEG_AudioReader::openMedia()
{
    // RLP_LOG_DEBUG(_srcFilePath );

    if (_srcFilePath == "")
    {
        RLP_LOG_ERROR("No input file, aborting")
        return false;
    }
    
    // Create decode context
    if (initializeCodecContext() >= 0)
    {
        initializeDestFormatContext();
        return true;
    }


    // RLP_LOG_DEBUG("DUMPING FORMAT: " << _srcFilePath.toStdString().c_str() );

    // dump input information to stderr
    // av_dump_format(_fmtCtx, 0, _srcFilePath.toStdString().c_str(), 0);

    return false;
}


int
RLFFMPEG_AudioReader::initializeCodecContext()
{
    // RLP_LOG_DEBUG("");

    int ret;
    AVStream *st;
    // AVCodecContext *dec_ctx = NULL;
    const AVCodec *dec = NULL;
    AVDictionary *opts = NULL;

    int openResult = 0;


    QByteArray mpathba = _srcFilePath.toLocal8Bit();
    const char* srcPath = mpathba.data();

    // open input file, and allocate format context
    if (avformat_open_input(&_fmtCtx, srcPath, NULL, NULL) < 0)
    {
        fprintf(stderr, "Could not open source file %s\n", _srcFilePath.toStdString().c_str());
        // exit(1);

        openResult = -1;
    }

    // retrieve stream information
    if (avformat_find_stream_info(_fmtCtx, NULL) < 0)
    {
        fprintf(stderr, "Could not find stream information\n");
        // exit(1);

        openResult = -1;
    }


    ret = av_find_best_stream(_fmtCtx, _AVMEDIA_TYPE, -1, -1, NULL, 0);
    if (ret < 0)
    {
        fprintf(stderr, "Could not find %s stream in input file '%s'\n",
                av_get_media_type_string(_AVMEDIA_TYPE), _srcFilePath.toStdString().c_str());
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

    }

    return 0;

}


void
RLFFMPEG_AudioReader::initializeScalars()
{
    // Needed for initializeScalars() in base class since it calls getPts()
    // which might need the input sample rate
    //
    _inSampleRate = _streamDecCtx->sample_rate; // _->codec->sample_rate;



    // RLP_LOG_DEBUG("RLFFMPEG_ReaderBase::initializeScalars()" );

    uint64_t currPts;
    long ptsSum = 0;
    long sampleSum = 0;

    uint64_t lastPts = 0;

    for (int f=0; f!=13; ++f)
    {

        RLFFMPEG_PacketReadResult p_ret = readNextPacket();

        /*
        RLP_LOG_DEBUG("RLFFMPEG_ReaderBase::initializeScalars() : "
                  << "[ " << _readerType.toStdString().c_str() << " ] "
                  << "f: " << f
                  << " pts: " << p_ret.pts_result
                  << " dts: " << p_ret.dts_result
                  << " nb_samples: "<< p_ret.av_frame->nb_samples );
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

    }

    _ptsIncrement = round(float(ptsSum) / 10.0);

    _sampleIncrement = round(float(sampleSum) / 10);

    if (!_properties->contains("video.frame_count"))
    {
        RLP_LOG_DEBUG("NO FRAMECOUNT" );
    }

    // _ptsMax = getPts(_properties->value("video.frame_count").value<float>());
    // _ptsMax = getPts(getProperty<float>("video.frameCount"));

    QVariant ptsi;
    ptsi.setValue(_ptsIncrement);

    QVariant ptso;
    ptso.setValue(_ptsOffset);

    _properties->insert(_readerType + ".pts_increment", ptsi);
    _properties->insert(_readerType + ".pts_offset", ptso);

    // RLP_LOG_DEBUG(
    //           " pts sum: " << ptsSum
    //           << " pts offset: " << _ptsOffset
    //           << " pts increment: " << _ptsIncrement
    //           << " pts max: " << _ptsMax
    //           << " sample inc: " << _sampleIncrement );



    // TODO FIXME: WTF - seek/delay compensation scalar determined by hand
    //
    if (_properties->value("audio.codec.name").value<QString>() == QString("aac"))
    {
        _timeMultiplier = 1.0001;
    }


    // RLP_LOG_DEBUG(
    //           " seek time multiplier: " << _timeMultiplier
    //           << " audio sample rate: " << _inSampleRate
    //           << " output sample num: " << _sampleIncrement
    //           );


}


void
RLFFMPEG_AudioReader::initializeDestFormatContext()
{
    // RLP_LOG_DEBUG("");

    int ret;

    // TODO FIXME:
    // read from properties to set:
    // audio.output.channels -> AV_CH_LAYOUT_STEREO
    // audio.output.bitdepth -> AV_SAMPLE_FMT_S16
    //
    //
    /**
     * Create a resampler context for the conversion.
     * Set the conversion parameters.
     * Default channel layouts based on the number of channels
     * are assumed for simplicity (they are sometimes not detected
     * properly by the demuxer and/or decoder).
     */
    int n_channels = _streamDecCtx->ch_layout.nb_channels;
    AVChannelLayout* ch_layout;

    av_channel_layout_default(ch_layout, 2);

    AVChannelLayout out_layout = AV_CHANNEL_LAYOUT_STEREO;

    int swrResult = swr_alloc_set_opts2(
        &_swrCtx,
        &out_layout,
        AV_SAMPLE_FMT_S16,
        _outputSampleRate, // 48000,
        ch_layout,
        _streamDecCtx->sample_fmt,
        _streamDecCtx->sample_rate,
        0, NULL
    );

    if (swrResult != 0)
    {
        fprintf(stderr, "Could not allocate resample context\n");
        // return AVERROR(ENOMEM);
    }

    /**
    * Perform a sanity check so that the number of converted samples is
    * not greater than the number of samples to be converted.
    * If the sample rates differ, this case has to be handled differently
    */
    //av_assert0(output_codec_context->sample_rate == input_codec_context->sample_rate);

    int error;

    /** Open the resampler with the specified parameters. */
    if ((error = swr_init(_swrCtx)) < 0)
    {
        fprintf(stderr, "Could not open resample context\n");
        swr_free(&_swrCtx);
        // return error;
    }



    double duration = _properties->value("audio.duration").value<double>();
    double totalOutputSamples = _outputSampleRate * duration;

    RLP_LOG_DEBUG(
              " out bitdepth: " << _outputBitDepth
              << " out channels: " << _outputChannelCount
              << " out samplerate: " << _outputSampleRate
              << " total samples: " << totalOutputSamples
              << " total duration: " << duration << " secs"
              );

}


uint64_t
RLFFMPEG_AudioReader::getPts(long frameNum)
{
    // TODO FIXME: NOT USED?

    // RLP_LOG_DEBUG(frameNum << " with rate " << _properties->value("video.frame_rate").toDouble())

    // Get timestamp of this frame (in seconds)
    double preSeconds = (double(frameNum) / _properties->value("video.frame_rate").toDouble()); //  * 0.995;
    double postSeconds = preSeconds * _timeMultiplier;

    long int audio_pts = round((_inSampleRate / (double)_sampleIncrement) * postSeconds * _ptsIncrement); //  - _ptsOffset;


    // RLP_LOG_DEBUG("RLFFMPEG_AudioReader::getPts(): seconds: " << preSeconds << " post seconds: " << postSeconds << " sample rate: " << _inSampleRate << " sample inc: " << _sampleIncrement << " dts inc: " << _ptsIncrement );

    if (_sampleIncrement == _ptsIncrement)
    {

        return audio_pts;

    } else if (
        _properties->contains("video.dtsIncrement") &&
        _properties->contains("video.dtsOffset"))
    {

        int videoPtsIncrement = _properties->value("video.dtsIncrement").value<int>();
        int videoPtsOffset = _properties->value("video.dtsOffset").value<int>();

        // frameNum = round((double)frameNum * 1.18);

        // RLP_LOG_DEBUG("RLFFMPEG_AudioReader::getPts() USING VIDEO DTS VALUES" );
        return ((frameNum - 1) * videoPtsIncrement); // + videoPtsOffset;

    } else
    {

        // RLP_LOG_DEBUG("RLFFMPEG_AudioReader::getPts() FAILED!!!" );
        return 0;
    }
}


RLFFMPEG_PacketReadResult
RLFFMPEG_AudioReader::seekToFrame(long frameNum)
{
    RLP_LOG_DEBUG(frameNum << _properties->value("video.frame_rate").toDouble())

    double preSeconds = (double(frameNum) / _properties->value("video.frame_rate").toDouble()); //  * 0.995;

    int64_t timestamp = preSeconds / av_q2d(_fmtCtx->streams[_streamIdx]->time_base);


    // RLP_LOG_DEBUG("SEEK SECONDS: " << preSeconds << " TIMESTAMP: " << timestamp );


    int ret = avformat_seek_file(_fmtCtx, _streamIdx, 0, timestamp, timestamp, 0);
    avcodec_flush_buffers(_streamDecCtx);


    // AVFrame* frame = av_frame_alloc();
    RLFFMPEG_PacketReadResult p_ret = readNextPacket();

    // RLP_LOG_DEBUG("seek ret: " << ret << " pts: " << p_ret.pts_result );
              // << " dts: " << p_ret.dts_result


    uint64_t currPts = p_ret.pts_result;

    while ((currPts + _ptsIncrement) < timestamp) {

        // RLP_LOG_DEBUG("Marching forward - " << currPts );

        av_frame_unref(p_ret.av_frame);
        av_frame_free(&p_ret.av_frame);

        p_ret = readNextPacket();
        currPts = p_ret.pts_result;

    }


    // RLP_LOG_DEBUG("Ending on " << p_ret.pts_result );

    return p_ret;
}



bool
RLFFMPEG_AudioReader::getFrame(AVPacket* pkt, AVFrame* frame)
{
    if(avcodec_send_packet(_streamDecCtx, pkt) < 0){
        return false;
    }

    if(avcodec_receive_frame(_streamDecCtx, frame) < 0){
        return false;
    }

    return true;

}


// uint64_t
RLFFMPEG_PacketReadResult
RLFFMPEG_AudioReader::readNextPacket()
{
    // RLP_LOG_DEBUG("RLFFMPEG_AudioReader::readNextPacket() " );

    RLFFMPEG_PacketReadResult p_ret;
    p_ret.dts_result = 0;
    p_ret.pts_result = 0;
    p_ret.return_status = DS_Buffer::READ_UNKNOWN;
    p_ret.nb_samples = 0;
    p_ret.av_frame = av_frame_alloc();

    int got_frame = false;
    while (!got_frame)
    {

        // RLP_LOG_DEBUG("readNextPacket() looping in while" );

        AVPacket my_pkt;
        int arf_ret = av_read_frame(_fmtCtx, &my_pkt);

        p_ret.dts_result = my_pkt.dts;
        p_ret.pts_result = my_pkt.pts;

        // RLP_LOG_DEBUG("av_read_frame(): " << arf_ret << " streamidx: " << my_pkt.stream_index << "dts: " << my_pkt.dts );

        if (arf_ret >= 0) {

            if (my_pkt.stream_index == _streamIdx) {

                if(!getFrame(&my_pkt, p_ret.av_frame)) {

                    // RLP_LOG_DEBUG("LOOPING IN GET FRAME" );

                    p_ret.return_status = DS_Buffer::READ_ERROR;
                    RLP_LOG_ERROR("GET FRAME FAILED" );
                    // exit(1);
                    continue;
                }

                // int dret = avcodec_decode_audio4(_streamDecCtx, frame, &got_frame, m_pkt);
                // if (dret < 0) {
                //    fprintf(stderr, "Error decoding audio frame (%s)\n", av_err2str(dret));
                //    return ret;
                // }

                got_frame = true;

            // break;
            }

        } else {

            // fprintf(stderr, "Error decoding frame - %i - %s\n", arf_ret, av_err2str(arf_ret));
            // fprintf(stderr, "Error decoding frame - %i - %s\n", arf_ret);
            // char err_str[4096];
            av_make_error_string(_errStr, 4096, arf_ret);

            RLP_LOG_ERROR("Error decoding frame - " << _errStr );

            got_frame = true;

            p_ret.return_status = DS_Buffer::READ_EOF;

        }

        av_packet_unref(&my_pkt);

    }

    // RLP_LOG_DEBUG("readNextPacket(): " << p_ret.dts_result << " return result: " << p_ret.return_status );

    return p_ret;
}


void
RLFFMPEG_AudioReader::readAudio(DS_TimePtr timeInfo, DS_BufferPtr inBuffer)
{
    QMutexLocker ll(&_lock);

    AUDIO_Time* atime = static_cast<AUDIO_Time* >(timeInfo.get());

    // RLP_LOG_DEBUG(""
    //     << _srcFilePath
    //     << timeInfo->mediaVideoFrame()
    //     << timeInfo->videoFrame()
    //     << atime->bufferIndex()
    //     << atime->samples()
    //     << atime->bytes()
    //     << " last pos: " << _audioStartVideoFrame
    // )

    // _frame = av_frame_alloc();

    RLFFMPEG_PacketReadResult p_ret;


    if (std::abs(long(timeInfo->mediaVideoFrame() - _audioStartVideoFrame)) > 2)
    {
        // RLP_LOG_DEBUG("seek required to " << timeInfo->mediaVideoFrame() );

        if ((timeInfo->mediaVideoFrame() == 0) || (timeInfo->mediaVideoFrame() == 1))
        {
            resetAll();
            p_ret = readNextPacket();

        } else
        {
            p_ret = seekToFrame(timeInfo->mediaVideoFrame());
        }

        _audioStartVideoFrame = timeInfo->mediaVideoFrame();


    } else
    {
        // RLP_LOG_DEBUG("reading next packet to " << timeInfo->mediaVideoFrame() );

        p_ret = readNextPacket();

        _audioStartVideoFrame = timeInfo->mediaVideoFrame();
    }

    if (p_ret.return_status == (int)DS_Buffer::READ_EOF)
    {
        qlonglong frameCount = _properties->value("media.frame_count").toLongLong();
        qlonglong mvf = timeInfo->mediaVideoFrame();

        if ((mvf >= frameCount) ||
            ((mvf + 1) == frameCount))
        {
            p_ret.return_status = DS_Buffer::READ_EOF_OK;

        } else
        {
            RLP_LOG_WARN("Irregular EOF detected on frame" << timeInfo->mediaVideoFrame() << " frameCount:" << frameCount << _srcFilePath)
        }

        // TODO: copy from empty buffer?
    }

    inBuffer->getBufferMetadata()->insert("audio.read_result", p_ret.return_status);
    
    if (p_ret.return_status == DS_Buffer::READ_EOF || p_ret.return_status == DS_Buffer::READ_EOF_OK)
    {
        return;
    }


    int sample_count = 0;
    int size_count = 0;



    // RLP_LOG_DEBUG("frame nb samples: " << frame->nb_samples );


    // compute the number of converted samples: buffering is avoided
    // ensuring that the output buffer will contain at least all the
    // converted input samples
    int dst_nb_samples = av_rescale_rnd(
        p_ret.av_frame->nb_samples,
        _outputSampleRate,
        _streamDecCtx->sample_rate,
        AV_ROUND_UP);

    // RLP_LOG_DEBUG("writing" << dst_nb_samples << "samples")

    // DANGER?
    DS_AudioBuffer* destBuffer = (DS_AudioBuffer*)inBuffer.get();
    destBuffer->reallocate(_outputBitDepth, _outputChannelCount, dst_nb_samples);
    // destBuffer->reallocate((outputBitdepth() / 8), outputChannelCount(), p_ret.av_frame->nb_samples);

    uint8_t* destBufferData = (uint8_t*)destBuffer->data();
    // int16_t* destBufferData = (int16_t*)inBuffer->data();



    // RLP_LOG_DEBUG("dst_nb_samples: " << dst_nb_samples );

    // memcpy(destBufferData, p_ret.av_frame->data[0], p_ret.av_frame->nb_samples * 4);

    /*
    for (int x=0; x != p_ret.av_frame->nb_samples; ++x) {
        RLP_LOG_DEBUG("read(): " << timeInfo->mediaVideoFrame() << " " << x << " " << destBufferData[x * 2] );
    }
    */



    int got_samples = swr_convert(
        _swrCtx,
        &destBufferData,
        dst_nb_samples, // 4096, // out_samples,
        (const uint8_t **)p_ret.av_frame->data,
        p_ret.av_frame->nb_samples);


    // int got_samples = 1024;

    // int got_samples = p_ret.av_frame->nb_samples;

    av_frame_unref(p_ret.av_frame);
    av_frame_free(&p_ret.av_frame);


    // destBuffer->setDataSize(got_samples * outputChannelCount() * (outputBitdepth() / 8));
    destBuffer->setDataSize(got_samples * _outputChannelCount * _outputBitDepth);


    // RLP_LOG_DEBUG("got samples: " << got_samples );
    

}


void
RLFFMPEG_AudioReader::scheduleReadTask(
    DS_TimePtr timeInfo,
    DS_BufferPtr destBuffer,
    DS_Node::NodeDataType dataType,
    bool optional)
{

    QRunnable* task = new RLFFMPEG_AudioReadTask(this, timeInfo, destBuffer);
    QThreadPool* tpool = _properties->value("audio.plugin").
                                      value<RLFFMPEG_Plugin *>()->
                                      getReadThreadPool(dataType);

    if (optional) {
        tpool->tryStart(task);
    } else {
        tpool->start(task);
    }


}