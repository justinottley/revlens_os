
#include "RlpExtrevlens/RLOIIO/Plugin.h"
#include "RlpExtrevlens/RLOIIO/FrameBuffer.h"
#include "RlpExtrevlens/RLOIIO/VideoReader.h"

#include "RlpCore/SEQ/Util.h"

#include <OpenImageIO/imageio.h>

RLP_SETUP_EXT_LOGGER(RLOIIO, Plugin)

RLOIIO_Plugin::RLOIIO_Plugin():
    MEDIA_Plugin("OpenImageIO"),
    _readerThreads(4)
{
    QThreadPool* videoPool = new QThreadPool();
    videoPool->setMaxThreadCount(_readerThreads);
    
    _readThreadPoolMap.insert(DS_Node::DATA_VIDEO, videoPool);
    
    RLP_LOG_INFO("Read thread pool initialized - reader threads: "
        << _readerThreads);
    
}


DS_FrameBufferPtr
RLOIIO_Plugin::getOrCreateFrameBuffer(QObject* owner, QVariantMap frameInfo, bool /* forceNew */)
{
    RLP_LOG_DEBUG("")
    return DS_FrameBufferPtr(new RLOIIO_FrameBuffer(owner, frameInfo));
}


QVariantList
RLOIIO_Plugin::identifyMedia(QVariantMap mediaInput, DS_Node::NodeDataType dataType)
{
    QVariantMap properties;
    QVariantList result;

    properties.clear();
    result.clear();

    if (!mediaInput.contains("media.video"))
    {
        return result;
    }

    // only attempt to open frame sequence type component
    if (!mediaInput.contains("media.video.resolved_component"))
    {
        RLP_LOG_DEBUG("resolved_component not found, aborting")
        return result;
    }

    QString resolvedComponentType = mediaInput.value("media.video.resolved_component").toString();
    if (resolvedComponentType != "frames")
    {
        RLP_LOG_DEBUG("frame component not found, aborting")
        return result;
    }

    QString videoPath = mediaInput.value("media.video").toString();  // value<QString>("video_dir");
    RLP_LOG_DEBUG(videoPath);

    if ((videoPath.indexOf(".exr")) != -1)
    {
        RLP_LOG_DEBUG("Skipping EXR");
        return result;
    }

    // TODO FIXME: for some reason, some jpegs dont read into ImageInput correctly, they read broken. WHY?
    // Use the builtin reader instead
    if ((videoPath.indexOf(".jpg")) != -1)
    {
        RLP_LOG_DEBUG("Skipping Jpeg");
        return result;
    }

    if ((videoPath.indexOf(".jpeg")) != -1)
    {
        RLP_LOG_DEBUG("Skipping Jpeg");
        return result;
    }

    if ((videoPath.indexOf(".png")) != -1)
    {
        RLP_LOG_DEBUG("Skipping png");
        return result;
    }



        SEQ_SequencePtr seq = SEQ_Util::getSequenceFromTemplateString(videoPath);
    if (seq == nullptr) {
        RLP_LOG_DEBUG("no sequences found");
        return result;
    }
    
    // QDir dir = QDir(videoPath);
    // QFileInfoList files = dir.entryInfoList(QDir::Files);
    // uint numFiles = files.count();

    int numFrames = seq->size();
    
    RLP_LOG_DEBUG("sequence size: " << numFrames);
    
    if (numFrames <= 0) {
        
        RLP_LOG_ERROR("no files found!")
        return result;
    }
    
    // QVariant seqV;
    // seqV.setValue(seq);
    properties.insert("video.sequence", videoPath);
    
    properties.insert("video.start_frame", 1);
    properties.insert("video.end_frame", numFrames);
    
    QFileInfo firstFile = seq->at(0)->fileInfo();
    
    // Media name
    //
    QString mediaName = seq->itemTemplate()->prefix() + " " + seq->frange();
    
    properties.insert("media.name", firstFile.baseName());
    
    
    QString filePath = firstFile.absoluteFilePath();
    // "/home/justinottley/Videos/jpg_example/bbb_trailer_480p.0001.jpg"; // 
    
    RLP_LOG_DEBUG("identifying " << filePath);
    
    properties.insert("video.frame_count", numFrames);
    properties.insert("media.frame_count", numFrames);


    //
    // OIIO STUFF
    //

    auto in = OIIO::ImageInput::open(filePath.toUtf8().constData());
    if (!in) {
        RLP_LOG_DEBUG("Error opening file")
        return result;
    }

    RLP_LOG_DEBUG("Open OK")

    const OIIO::ImageSpec& spec = in->spec();
    int width = spec.width;
    int height = spec.height;
    int channelCount = spec.nchannels;
    int depth = spec.depth;
    int bitsPerSample = spec.get_int_attribute("oiio:BitsPerSample", 0);

    const OIIO::TypeDesc td = spec.format;

    int pixelTypeDepth = 1;
    if (td == OIIO::TypeDesc::UINT16)
    {
        // pixelTypeDepth = 2;
    }

    int byteCount = width * height * channelCount * pixelTypeDepth;

    QString formatName(in->format_name());

    QVariantMap frameInfo;
    frameInfo.insert("width", width);
    frameInfo.insert("height", height);
    frameInfo.insert("channelCount", channelCount);
    frameInfo.insert("pixelSize", pixelTypeDepth);
    frameInfo.insert("byteCount", byteCount);

    QVariantList frameInfoList;
    frameInfoList.push_back(frameInfo);
    properties.insert("video.frame_info", frameInfoList);
    properties.insert("video.frame_info.one", frameInfo);
    properties.insert("video.format_name", formatName);

    RLP_LOG_DEBUG("Identified OIIO:" << formatName)

    RLP_LOG_DEBUG("Width:" << width)
    RLP_LOG_DEBUG("Height:" << height)
    RLP_LOG_DEBUG("Channels:" << channelCount)
    RLP_LOG_DEBUG("Format:" << spec.format.c_str())
    RLP_LOG_DEBUG("Bits Per Sample:" << bitsPerSample)

    properties.insert("video.sequence", videoPath);
    properties.insert("video.format.type", MEDIA_Plugin::VIDEO_FRAMES);
    properties.insert("video.format.reader", name());
    properties.insert("video.frame_rate", 24);
    // properties.insert("video.colourspace", "linear");

    properties.insert("media_type", MEDIA_Plugin::VIDEO_FRAMES);

    result.push_back(properties);

    in->close();

    return result;
}


DS_NodePtr
RLOIIO_Plugin::createVideoReader(QVariantMap* properties)
{
    RLP_LOG_DEBUG("")

    DS_NodePtr node(new RLOIIO_VideoReader(properties));
    return node;
}



RLOIIO_ReadTask::RLOIIO_ReadTask(RLOIIO_VideoReader* reader,
                                 DS_TimePtr timeInfo,
                                 DS_BufferPtr destFrame) :
    _reader(reader),
    _timeInfo(timeInfo),
    _destFrame(destFrame)
{
}


void
RLOIIO_ReadTask::run()
{
    // std::cout << "RLEXRMEDIA_ReadTask::run()" << std::endl;
    //
    //if ((_reader == nullptr) || (_reader == NULL)) {
    //    return;
    //}
    
    _reader->readVideo(_timeInfo, _destFrame);
}