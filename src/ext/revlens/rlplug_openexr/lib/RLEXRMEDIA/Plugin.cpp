//
//  Copyright (c) 2014 Justin Ottley. All rights reserved.
//


#include "RlpExtrevlens/RLEXRMEDIA/Plugin.h"
#include "RlpExtrevlens/RLEXRMEDIA/VideoReader.h"

#include "RlpExtrevlens/RLEXRDS/FrameBuffer.h"

#include "RlpExtrevlens/RLEXRMEDIA/ExrIncludes.h"

#include "RlpCore/SEQ/Init.h"



RLP_SETUP_EXT_LOGGER(RLEXRMEDIA, Plugin)

RLEXRMEDIA_Plugin::RLEXRMEDIA_Plugin() :
    MEDIA_Plugin(QString("OpenEXR")),
    _readerThreads(4),
    _imfThreads(8)
{
    
    QThreadPool* videoPool = new QThreadPool();
    videoPool->setMaxThreadCount(_readerThreads);
    
    Imf::setGlobalThreadCount(_imfThreads);
    
    _readThreadPoolMap.insert(DS_Node::DATA_VIDEO, videoPool);
    
    RLP_LOG_INFO("Read thread pool initialized - reader threads: "
        << _readerThreads << " Imf threads: " << _imfThreads);
    
}


DS_FrameBufferPtr
RLEXRMEDIA_Plugin::getOrCreateFrameBuffer(QObject* owner, QVariantMap frameInfo, bool /* forceNew */)
{
    // RLP_LOG_DEBUG("")
    return DS_FrameBufferPtr(new RLEXRDS_FrameBuffer(owner, frameInfo));
}


QVariantList
RLEXRMEDIA_Plugin::identifyMedia(QVariantMap mediaInput, DS_Node::NodeDataType dataType)
{
    QVariantMap properties;
    QVariantList result;
    
    properties.clear();
    result.clear();


    if (!mediaInput.contains("media.video"))
    {
        return result;
    }
        
    QString videoPath = mediaInput.value("media.video").value<QString>();  // value<QString>("video_dir");
    RLP_LOG_DEBUG(videoPath);
    
    if ((videoPath.indexOf(".exr"))  == -1)
    {
        RLP_LOG_DEBUG("EXR extension not found, skipping");
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
    
    RLP_LOG_DEBUG("identifying " << filePath);
    
    properties.insert("video.frame_count", numFrames);
    properties.insert("media.frame_count", numFrames);

    // EXR STUFF
    
    QByteArray mpathba = filePath.toLocal8Bit();
    const char* srcPath = mpathba.data();

    if (!Imf::isOpenExrFile(srcPath)) {
        
        RLP_LOG_DEBUG("Not an OpenEXR file: " << srcPath)
        return result;
    }

    Imf::RgbaInputFile file(srcPath);
    int version = file.version();

    RLP_LOG_DEBUG("Open OK:" << srcPath)
    RLP_LOG_DEBUG("EXR Version: " << version);

    Imath::Box2i dw = file.dataWindow();

    int width  = dw.max.x - dw.min.x + 1;
    int height = dw.max.y - dw.min.y + 1;

    RLP_LOG_DEBUG("DataWindow: " << width << " " << height)
    
    // NOTE: only one part exr here
    const Imf::Header& h = file.header();
    
    Imf::Header::ConstIterator i;
    
    // TODO FIXME: hardcoded
    int channelCount = 4;
    int exrChannelCount = 0; // actual number of channels found
    int pixelTypeDepth = 0;

    // TODO FIXME: Detect R, G, B named channels, and A optionally
    //
    for (i = h.begin(); i != h.end(); ++i) {
        
        const Imf::Attribute *a = &i.attribute();

        RLP_LOG_DEBUG(i.name() << " (type " << a->typeName() << ")")
        
        if (const Imf::ChannelListAttribute* chl_attr = 
                dynamic_cast<const Imf::ChannelListAttribute* >(a)) {
            
            Imf::ChannelList chlist = chl_attr->value();
            Imf::ChannelList::ConstIterator chi;
            for (chi = chlist.begin(); chi != chlist.end(); ++chi) {
                
                Imf::PixelType pt = chi.channel().type;
                
                if (pt == Imf::HALF) {
                    
                    RLP_LOG_DEBUG("HALF detected")
                    pixelTypeDepth = 2; // sizeof(half);

                } else if (pt == Imf::FLOAT) {

                    RLP_LOG_DEBUG("FLOAT detected")
                    pixelTypeDepth = 4; // sizeof(float);
                    
                }

                exrChannelCount++;

                RLP_LOG_DEBUG(chi.name() << " " << (int)chi.channel().type)
                
            }
        }
        
    }
    
    
    int byteCount = width * height * channelCount * pixelTypeDepth;

    RLP_LOG_DEBUG("channel count:" << exrChannelCount)
    RLP_LOG_DEBUG("byteCount: " << byteCount)



    // TODO CBB: proper sequence parsing
    // properties.set("frameCount", numFiles);
    
    // properties.insert("video.startFrame", 1);
    // properties.insert("video.endFrame", numFiles);
    
    // NOTE:
    // info for one frame only - assuming for now all the frames are identical
    // TODO: identify up front if the frames are not identical and store
    // info forr each frame
    //
    
    QVariantMap frameInfo;
    
    frameInfo.insert("width", width);
    frameInfo.insert("height", height);
    frameInfo.insert("pixelSize", pixelTypeDepth);
    frameInfo.insert("channelCount", channelCount);
    frameInfo.insert("exr.channelCount", exrChannelCount);
    frameInfo.insert("byteCount", byteCount);

    frameInfo.insert("dw.min.x", dw.min.x);
    frameInfo.insert("dw.min.y", dw.min.y);
    frameInfo.insert("dw.max.x", dw.max.x);
    frameInfo.insert("dw.max.y", dw.max.y);
    
    QVariantList frameInfoList;
    frameInfoList.push_back(frameInfo);

    properties.insert("video.frame_info", frameInfoList);
    properties.insert("video.frame_info.one", frameInfo);

    RLP_LOG_DEBUG("Identified EXR FRAMES")

    properties.insert("video.format.type", MEDIA_Plugin::VIDEO_FRAMES);
    properties.insert("video.format.reader", name());
    properties.insert("video.colourspace", "linear");

    float frameRate = 24.0;
    if (mediaInput.contains("media.frame_rate")) {
        float frameRate = mediaInput.value("media.frame_rate").value<float>();  // value<QString>("video_dir");    
    }

    properties.insert("video.frame_rate", frameRate);
    properties.insert("media_type", MEDIA_Plugin::VIDEO_FRAMES);

    result.push_back(properties);

    return result;
}


DS_NodePtr
RLEXRMEDIA_Plugin::createVideoReader(QVariantMap* properties)
{
    DS_NodePtr node(new RLEXRMEDIA_VideoReader(properties));
    return node;
}


//


RLEXRMEDIA_ReadTask::RLEXRMEDIA_ReadTask(RLEXRMEDIA_VideoReader* reader,
                                             DS_TimePtr timeInfo,
                                             DS_BufferPtr destFrame) :
    _reader(reader),
    _timeInfo(timeInfo),
    _destFrame(destFrame)
{
}


void
RLEXRMEDIA_ReadTask::run()
{
    // std::cout << "RLEXRMEDIA_ReadTask::run()" << std::endl;
    //
    //if ((_reader == nullptr) || (_reader == NULL)) {
    //    return;
    //}
    
    _reader->readVideo(_timeInfo, _destFrame);
}
