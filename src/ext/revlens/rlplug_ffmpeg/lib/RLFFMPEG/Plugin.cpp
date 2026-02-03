//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include "RlpExtrevlens/RLFFMPEG/VideoReader3.h"
#include "RlpExtrevlens/RLFFMPEG/BypassVideoReader.h"
#include "RlpExtrevlens/RLFFMPEG/StreamingVideoReader.h"
#include "RlpExtrevlens/RLFFMPEG/AudioReader2.h"
#include "RlpExtrevlens/RLFFMPEG/Plugin.h"

extern "C"
{

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

}

RLP_SETUP_LOGGER(ext, RLFFMPEG, Plugin)

RLFFMPEG_Plugin::RLFFMPEG_Plugin():
    MEDIA_Plugin("FFmpeg")
{

    QThreadPool* videoPool = new QThreadPool();
    videoPool->setMaxThreadCount(1);

    QThreadPool* audioPool = new QThreadPool();
    audioPool->setMaxThreadCount(1);

    _readThreadPoolMap.insert(DS_Node::DATA_VIDEO, videoPool);
    _readThreadPoolMap.insert(DS_Node::DATA_AUDIO, audioPool);


    RLP_LOG_DEBUG("");
}


bool
RLFFMPEG_Plugin::identifyAudio(
    QString srcFilePath,
    QVariantMap &mediaIn,
    QVariantList* result)
{
    RLP_LOG_DEBUG(srcFilePath);

    // AVFormatContext* fmt_ctx = NULL;
    // const char* test_media = "C:\\Users\\justi\\dev\\revlens_root\\examples\\all-about-nina-trailer-1_h480p.mov";

    QByteArray mpathba = srcFilePath.toLocal8Bit();
    const char* srcPath = mpathba.data();

    AVFormatContext* fmt_ctx = NULL;
    if (avformat_open_input(&fmt_ctx, srcPath, NULL, NULL) < 0)
    {
        return false;
    }

    // retrieve stream information
    if (avformat_find_stream_info(fmt_ctx, NULL) < 0)
    {
        fprintf(stderr, "Could not find stream information\n");

        avformat_close_input(&fmt_ctx);
        return false;
    }

    int ret = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);

    if (ret < 0)
    {
        fprintf(stderr, "Could not find %s stream in input file '%s'\n",
                av_get_media_type_string(AVMEDIA_TYPE_AUDIO),
                srcPath);

        // avformat_close_input(&fmt_ctx);
        // return properties;

        return false;
    }
    else
    {

        int audioStreamIdx = ret;

        AVStream* st = fmt_ctx->streams[audioStreamIdx];

        /* find decoder for the stream */

        const AVCodec* dec = avcodec_find_decoder(st->codecpar->codec_id);
        if (!dec)
        {
            fprintf(stderr, "Failed to find %s codec\n",
                    av_get_media_type_string(AVMEDIA_TYPE_AUDIO));

            return false;
        }

        AVCodecContext* dec_ctx = avcodec_alloc_context3(dec);

        /* Copy codec parameters from input stream to output codec context */
        if ((ret = avcodec_parameters_to_context(dec_ctx, st->codecpar)) < 0)
        {
            fprintf(stderr, "Failed to copy %s codec parameters to decoder context\n",
                    av_get_media_type_string(AVMEDIA_TYPE_AUDIO));
            return ret;
        }

        AVDictionary* opts = NULL;
        if ((ret = avcodec_open2(dec_ctx, dec, &opts)) < 0)
        {
            fprintf(stderr, "Failed to open %s codec\n",
                    av_get_media_type_string(AVMEDIA_TYPE_AUDIO));

            // avformat_close_input(&fmt_ctx);
            // return properties;

            return false;
        }

        int64_t duration = fmt_ctx->duration;

        float start_time = 0.0;
        if (fmt_ctx->start_time != AV_NOPTS_VALUE)
        {
            start_time = float(fmt_ctx->start_time) / float(AV_TIME_BASE);
            duration = duration + fmt_ctx->start_time;
            // RLP_LOG_DEBUG("FOUND START TIME: " << start_time << " " << fmt_ctx->start_time);
        }

        float use_duration = st->duration;
        float calc_duration_in_secs = 0;

        if (st->duration < 0)
        {
            // RLP_LOG_DEBUG("invalid stream duration, using format duration");
            use_duration = fmt_ctx->duration;
            calc_duration_in_secs = float(use_duration) / float(AV_TIME_BASE);
        }
        else
        {
            calc_duration_in_secs = (float(use_duration) * float(st->time_base.num)) / float(st->time_base.den);
        }

        qint64 audioFrames = int(rint(calc_duration_in_secs * dec_ctx->sample_rate));

        int outputSampleRate = mediaIn.value("audio.output.sampleRate").value<int>();
        qint64 totalOutputSamples = calc_duration_in_secs * outputSampleRate;
        int n_channels = dec_ctx->ch_layout.nb_channels;

        RLP_LOG_VERBOSE("AUDIO time base: " << st->time_base.num << " " << st->time_base.den);
        RLP_LOG_VERBOSE("FMT DURATION: " << fmt_ctx->duration);
        RLP_LOG_VERBOSE("stream DURATION: " << st->duration);
        RLP_LOG_VERBOSE("stream START TIME: " << st->start_time);
        RLP_LOG_VERBOSE("Calculated duration: " << calc_duration_in_secs);
        RLP_LOG_VERBOSE("FOUND AUDIO TRACK at index " << audioStreamIdx);
        RLP_LOG_VERBOSE("audio codec: " << dec_ctx->codec->name);
        RLP_LOG_VERBOSE("channels: " << n_channels);
        RLP_LOG_VERBOSE("sample rate: " << dec_ctx->sample_rate);
        RLP_LOG_VERBOSE("bit rate: " << st->codecpar->bit_rate);
        RLP_LOG_VERBOSE("Calculated input samples: " << audioFrames);
        RLP_LOG_VERBOSE("Calculated output samples: " << totalOutputSamples);
        RLP_LOG_VERBOSE("Output sample rate: " << outputSampleRate);

        QVariantMap properties;

        // properties.insert("media.audio", srcFilePath);

        properties.insert("audio.format.type", MEDIA_Plugin::AUDIO_CONTAINER);
        properties.insert("audio.format.reader", name());
        properties.insert("audio.codec.name", QString(dec_ctx->codec->name));
        properties.insert("audio.duration", calc_duration_in_secs);

        properties.insert("audio.output.totalSamples", totalOutputSamples);

        properties.insert("media_type", MEDIA_Plugin::AUDIO_CONTAINER);

        result->push_back(properties);

        avcodec_free_context(&dec_ctx);
    }

    avformat_close_input(&fmt_ctx);

    return true;
}


bool
RLFFMPEG_Plugin::identifyVideo(
    QString srcFilePath,
    QVariantMap &mediaIn,
    QVariantList* result)
{

    // TODO FIXME: probably better to accept only media containers
    // and not frame sequences

    if (srcFilePath.endsWith(".exr"))
    {
        // RLP_LOG_DEBUG("Skipping identify, found EXR")
        return result;
    }

    if (srcFilePath.endsWith(".png"))
    {
        // RLP_LOG_DEBUG("Skipping identify, found PNG")
        return result;
    }

    if (srcFilePath.endsWith(".tif") || srcFilePath.endsWith(".tiff"))
    {
        // RLP_LOG_DEBUG("Skipping identify, found TIF")
        return result;
    }

    if (srcFilePath.endsWith(".psd") || srcFilePath.endsWith("psb"))
    {
        // RLP_LOG_DEBUG("Skipping identify, found PSD/PSB")
        return result;
    }

    // RLP_LOG_DEBUG(srcFilePath)

    QByteArray mpathba = srcFilePath.toLocal8Bit();
    const char* srcPath = mpathba.data();

    AVDictionary *opts = NULL;
    av_dict_set(&opts, "rtsp_transport", "tcp", 0);

    AVFormatContext* fmt_ctx = NULL;
    if (avformat_open_input(&fmt_ctx, srcPath, NULL, &opts) < 0)
    {
        return false;
    }

    // retrieve stream information
    if (avformat_find_stream_info(fmt_ctx, NULL) < 0)
    {
        fprintf(stderr, "Could not find stream information\n");

        avformat_close_input(&fmt_ctx);
        return false;
    }

    int ret = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);

    if (ret < 0)
    {
        fprintf(stderr, "Could not find %s stream in input file '%s'\n",
                av_get_media_type_string(AVMEDIA_TYPE_VIDEO),
                srcPath);

        // avformat_close_input(&fmt_ctx);
        // return properties;

        // return false;
    }
    else
    {

        int videoStreamIdx = ret;

        AVStream* st = fmt_ctx->streams[videoStreamIdx];

        /* find decoder for the stream */

        const AVCodec* dec = avcodec_find_decoder(st->codecpar->codec_id);
        if (!dec)
        {
            fprintf(stderr, "Failed to find %s codec\n",
                    av_get_media_type_string(AVMEDIA_TYPE_VIDEO));

            return false;
        }

        AVCodecContext* dec_ctx = avcodec_alloc_context3(dec);

        /* Copy codec parameters from input stream to output codec context */
        if ((ret = avcodec_parameters_to_context(dec_ctx, st->codecpar)) < 0)
        {
            fprintf(stderr, "Failed to copy %s codec parameters to decoder context\n",
                    av_get_media_type_string(AVMEDIA_TYPE_VIDEO));
            return ret;
        }

        AVDictionary* opts = NULL;
        if ((ret = avcodec_open2(dec_ctx, dec, &opts)) < 0)
        {
            fprintf(stderr, "Failed to open %s codec\n",
                    av_get_media_type_string(AVMEDIA_TYPE_VIDEO));

            // avformat_close_input(&fmt_ctx);
            // return properties;

            return false;
        }

        QVariantMap properties;

        // AVRational av_framerate = st->codec->framerate;
        // float fl_framerate = float(av_framerate.num) / float(av_framerate.den);

        // RLP_LOG_DEBUG("RLFFMPEG_IOPlugin::identifyVideo()" );
        // RLP_LOG_DEBUG("AVStream num frames: " << st->nb_frames << " native framerate: " << fl_framerate );
        // RLP_LOG_DEBUG("AVFormatContext: duration: " << fmt_ctx->duration );

        int64_t duration = fmt_ctx->duration;

        // RLP_LOG_DEBUG("FMT DURATION: " << duration);
        // RLP_LOG_DEBUG("stream DURATION: " << st->duration);
        // RLP_LOG_DEBUG("stream START TIME: " << st->start_time);
        // RLP_LOG_DEBUG("stream TIME BASE: " << st->time_base.num << " " << st->time_base.den);

        float start_time = 0.0;
        if (fmt_ctx->start_time != AV_NOPTS_VALUE)
        {
            start_time = float(fmt_ctx->start_time) / float(AV_TIME_BASE);
            duration = duration + fmt_ctx->start_time;
            // RLP_LOG_DEBUG("FOUND START TIME: " << start_time << " " << fmt_ctx->start_time);
        }

        float use_duration = st->duration;
        float calc_duration_in_secs = 0;

        if (st->duration < 0)
        {
            // RLP_LOG_DEBUG("invalid stream duration, using format duration");
            use_duration = fmt_ctx->duration;
            calc_duration_in_secs = float(use_duration) / float(AV_TIME_BASE); // ??
        }
        else
        {
            calc_duration_in_secs = (float(use_duration) * float(st->time_base.num)) / float(st->time_base.den);
        }

        float calc_fps = float(st->avg_frame_rate.num) / float(st->avg_frame_rate.den);
        // int calc_frames = int(rint(calc_duration_in_secs * calc_fps));
        int calc_frames = int(rint((calc_duration_in_secs + start_time) * calc_fps));

        if ((st->avg_frame_rate.num == 0) && (st->avg_frame_rate.den == 0))
        {
            QFile f(srcFilePath);
            if (f.exists())
            {

                RLP_LOG_DEBUG("Single image detected");

                calc_frames = 1;
                calc_fps = 24;
            }
        }

        // RLP_LOG_DEBUG("Calculated duration: " << calc_duration_in_secs);
        // RLP_LOG_DEBUG("Avg fps: " << st->avg_frame_rate.num << " " << st->avg_frame_rate.den << " = " << calc_fps);
        // RLP_LOG_DEBUG("Calculated video frames: " << calc_frames);

        properties.insert("video.frame_count", calc_frames);
        properties.insert("video.frame_rate", calc_fps);
        properties.insert("video.codec", QString(dec->name));
        properties.insert("video.codec_long", QString(dec->long_name));
        properties.insert("video.start_time", start_time);
        
        // TODO FIXME: deprecate this, place in media.source_info instead
        properties.insert("media.frame_count", calc_frames);

        QVariantMap srcInfo = mediaIn.value("media.source_info").toMap();
        srcInfo.insert("media.frame_count", calc_frames);
        properties.insert("media.source_info", srcInfo);

        QVariantMap frameInfo;

        frameInfo.insert("width", dec_ctx->width);
        frameInfo.insert("height", dec_ctx->height);
        frameInfo.insert("pixelSize", 1);
        frameInfo.insert("bitsPerPixel", 32);
        frameInfo.insert("channelCount", 4);
        frameInfo.insert("bytePadding", AV_INPUT_BUFFER_PADDING_SIZE);

        frameInfo.insert("byteCount", (dec_ctx->width * dec_ctx->height * 4));

        QVariantList flist;
        flist.push_back(frameInfo);

        properties.insert("video.frame_info", flist);
        properties.insert("video.frame_info.one", frameInfo);

        // avcodec_close(st->codec);
        // avformat_close_input(&fmt_ctx);

        properties.insert("video.format.type", MEDIA_Plugin::VIDEO_CONTAINER);
        properties.insert("video.format.reader", name());
        properties.insert("video.colourspace", "sRGB");

        if (srcFilePath.startsWith("rtsp") || srcFilePath.startsWith("rtmp"))
        {
            RLP_LOG_INFO("Streaming Video Detected")
            properties.insert("video.streaming", true);
            properties.insert("media.frame_count", 100);
        }

        QVariantMap stream_result;

        properties.insert("media_type", MEDIA_Plugin::VIDEO_CONTAINER);

        // Arbitrary metadata in media
        //
        // encode with:
        // ffmpeg -metadata comment=VAL
        // where VAL is base64 encoded stringified JSON
        // only a few metadata keys are recognized by ffmpeg for quicktime
        // see https://wiki.multimedia.cx/index.php/FFmpeg_Metadata#QuickTime.2FMOV.2FMP4.2FM4A.2Fet_al.
        //

        /*

        AVDictionaryEntry* tag = NULL;
        while ((tag = av_dict_get(fmt_ctx->metadata, "", tag, AV_DICT_IGNORE_SUFFIX))) {

            if (strcmp(tag->key, "comment") == 0) {

                // Found comment
                printf("FOUND COMMENT METADATA: %s\n", tag->value);
                // QString comment(tag->value);
                QByteArray commentData = QByteArray::fromBase64(QString(tag->value).toLatin1());
                if (commentData.size() != 0) {
                    QJsonDocument resultData = QJsonDocument::fromJson(commentData);
                    QVariantMap data = resultData.object().toVariantMap();

                    properties.insert("video.metadata", data);

                } else {
                    RLP_LOG_ERROR("error reading comment metadata");

                }
            }

        }
        */

        result->push_back(properties);

        avcodec_free_context(&dec_ctx);

        av_dump_format(fmt_ctx, 0, srcPath, 0);
    }

    avformat_close_input(&fmt_ctx);

    return true;
}


bool
RLFFMPEG_Plugin::identifyNullVideoForAudioOnly(QVariantMap &mediaIn, QVariantList* result)
{

    QVariantMap properties;

    QVariantMap frameInfo;

    frameInfo.insert("width", 320);
    frameInfo.insert("height", 240);
    frameInfo.insert("pixelSize", 1);
    frameInfo.insert("bitsPerPixel", 32);
    frameInfo.insert("channelCount", 4);

    QVariantList flist;
    flist.push_back(frameInfo);

    properties.insert("video.frame_info", flist);
    properties.insert("video.frame_info.one", frameInfo);

    properties.insert("video.format.type", MEDIA_Plugin::VIDEO_CONTAINER);
    properties.insert("video.format.reader", name());
    properties.insert("video.colourspace", "sRGB");

    properties.insert("media_type", MEDIA_Plugin::VIDEO_CONTAINER);

    QVariantMap audioInfo = result->at(0).toMap();

    double audioDurationInSecs = audioInfo.value("audio.duration").toDouble();

    float defaultFrameRate = 24;
    qlonglong calcFrames = audioDurationInSecs * defaultFrameRate;

    RLP_LOG_DEBUG("GOT AUDIO DURATION:" << audioDurationInSecs << "secs /" << calcFrames << "frames")

    properties.insert("media.frame_count", calcFrames);
    properties.insert("video.frame_rate", defaultFrameRate);
    properties.insert("video.bypass", true);

    result->push_back(properties);

    return true;
}


QVariantList
RLFFMPEG_Plugin::identifyMedia(QVariantMap mediaIn, DS_Node::NodeDataType dataType)
{
    // RLP_LOG_DEBUG(dataType)

    QVariantList result;
    QString srcFilePath;

    bool videoResult = false;
    bool audioResult = false;

    if (mediaIn.contains("media.video") &&
        ((dataType == DS_Node::DATA_VIDEO) || (dataType == DS_Node::DATA_UNKNOWN)))
    {
        QString srcFilePath = mediaIn.value("media.video").value<QString>();
        videoResult = identifyVideo(srcFilePath, mediaIn, &result);
    }

    if (mediaIn.contains("media.audio") &&
        ((dataType == DS_Node::DATA_AUDIO) || (dataType == DS_Node::DATA_UNKNOWN)))
    {
        QString srcFilePath = mediaIn.value("media.audio").value<QString>();
        audioResult = identifyAudio(srcFilePath, mediaIn, &result);
    }

    if (audioResult && !videoResult && dataType == DS_Node::DATA_UNKNOWN)
    {
        identifyNullVideoForAudioOnly(mediaIn, &result);
    }

    return result;
}


DS_NodePtr
RLFFMPEG_Plugin::createVideoReader(QVariantMap* properties)
{
    // RLP_LOG_DEBUG("");
    // RLP_LOG_DEBUG(properties->keys());

    if (properties->contains("video.bypass"))
    {
        DS_NodePtr node(new RLFFMPEG_BypassVideoReader(properties));
        return node;
    }
    else if (properties->contains("video.streaming"))
    {
        RLP_LOG_DEBUG("Creating streaming video reader")
        DS_NodePtr node(new RLFFMPEG_StreamingVideoReader(properties));
        return node;
    }
    else 
    {
        DS_NodePtr node(new RLFFMPEG_VideoReader(properties));
        return node;
    }
}


DS_NodePtr
RLFFMPEG_Plugin::createAudioReader(QVariantMap* properties)
{
    // RLP_LOG_DEBUG("");

    DS_NodePtr node(new RLFFMPEG_AudioReader(properties));

    return node;
}


RLFFMPEG_VideoReadTask::RLFFMPEG_VideoReadTask(
    RLFFMPEG_VideoReader* reader,
    DS_TimePtr timeInfo,
    DS_BufferPtr destFrame
):
    _reader(reader),
    _timeInfo(timeInfo),
    _destFrame(destFrame)
{
}


void RLFFMPEG_VideoReadTask::run()
{
    _reader->readVideo(_timeInfo, _destFrame);
}


RLFFMPEG_AudioReadTask::RLFFMPEG_AudioReadTask(
    RLFFMPEG_AudioReader* reader,
    DS_TimePtr timeInfo,
    DS_BufferPtr destBuffer
): 
    _reader(reader),
    _timeInfo(timeInfo),
    _destFrame(destBuffer)
{
}


void RLFFMPEG_AudioReadTask::run()
{
    _reader->readAudio(_timeInfo, _destFrame);
}

