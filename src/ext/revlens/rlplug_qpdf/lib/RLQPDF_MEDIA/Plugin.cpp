
#include "RlpExtrevlens/RLQPDF_MEDIA/Plugin.h"
#include "RlpExtrevlens/RLQPDF_MEDIA/VideoReader.h"

#include "RlpRevlens/DS/QImageFrameBuffer.h"

RLP_SETUP_LOGGER(extrevlens, RLQPDF_MEDIA, Plugin)

RLQPDF_MEDIA_Plugin::RLQPDF_MEDIA_Plugin() :
    MEDIA_Plugin(QString("QPdf"))
{
    QThreadPool* videoPool = new QThreadPool();
    videoPool->setMaxThreadCount(1);

    _readThreadPoolMap.insert(DS_Node::DATA_VIDEO, videoPool);

    RLP_LOG_DEBUG("")
}


DS_FrameBufferPtr
RLQPDF_MEDIA_Plugin::getOrCreateFrameBuffer(QObject* owner, QVariantMap frameInfo, bool forceNew)
{
    // RLP_LOG_DEBUG("")

    if (!forceNew)
    {
        DS_FrameBufferPtr buf = getPreallocatedFrameBuffer(frameInfo);
        if (buf)
        {
            return buf;
        }
    }

    return DS_FrameBufferPtr(new DS_QImageFrameBuffer(owner, frameInfo, QImage::Format_RGB32));
}


QVariantList
RLQPDF_MEDIA_Plugin::identifyMedia(QVariantMap mediaInput, DS_Node::NodeDataType dataType)
{
    // RLP_LOG_DEBUG(mediaInput)

    QVariantMap properties;
    QVariantList result;

    properties.clear();
    result.clear();

    if (!mediaInput.contains("media.video"))
    {
        return result;
    }

    const QString filePath = mediaInput.value("media.video").value<QString>();  // value<QString>("video_dir");

    if ((filePath.indexOf(".pdf"))  == -1)
    {
        // RLP_LOG_DEBUG("PDF extension not found, skipping");
        return result;
    }

    // RLP_LOG_DEBUG("Attempting to identify:" << filePath)

    QPdfDocument pdfDoc;
    QPdfDocument::Error loadResult = pdfDoc.load(filePath);

    if (loadResult != QPdfDocument::Error::None)
    {
        // RLP_LOG_WARN("Error loading")
        return result;
    }

    int numPages = pdfDoc.pageCount();

    // Get aspect ratio of page, assume all the same
    QSizeF ppSize = pdfDoc.pagePointSize(0);

    int width = (int)ppSize.width();
    int height = (int)ppSize.height();
    int channelCount = 4;
    int pixelSize = 1;

    RLP_LOG_DEBUG("Load OK:" << filePath << "num. pages:" << numPages << "point size:" << ppSize)


    QImage image;

    
    // int width = image.width();
    // int height = image.height();
    // int channelCount = image.bitPlaneCount() / 8;
    // int pixelSize = image.depth() / 8;

    int byteCount = width * height * channelCount * pixelSize;

    float frameRate = 24.0;

    RLP_LOG_DEBUG("")
    RLP_LOG_DEBUG("  width:  " << width)
    RLP_LOG_DEBUG("  height: " << height)
    RLP_LOG_DEBUG("  channel count:" << channelCount)
    RLP_LOG_DEBUG("  pixelSize: " << pixelSize)
    RLP_LOG_DEBUG("  byteCount: " << byteCount)



    properties.insert("video.frame_count", numPages);
    // properties.insert("video.start_frame", 1);
    // properties.insert("video.end_frame", 1 + numPages);
    properties.insert("video.frame_rate", frameRate);

    // properties.insert("media.name", filePath.baseName());
    properties.insert("media.frame_count", numPages);

    // NOTE:
    // info for one frame only - assuming for now all the frames are identical
    // TODO: identify up front if the frames are not identical and store
    // info forr each frame
    //

    QVariantMap frameInfo;

    frameInfo.insert("width", width);
    frameInfo.insert("height", height);
    frameInfo.insert("pixelSize", pixelSize);
    frameInfo.insert("channelCount", channelCount); // TODO FIXME: what should this be? is this used?
    frameInfo.insert("byteCount", byteCount);



    QVariantList frameInfoList;
    frameInfoList.push_back(frameInfo);
    properties.insert("video.frame_info", frameInfoList);
    properties.insert("video.frame_info.one", frameInfo);

    properties.insert("video.format.type", MEDIA_Plugin::VIDEO_FRAMES);
    properties.insert("video.format.reader", name());

    // // properties.insert("video.colourspace", "linear");


    properties.insert("media_type", MEDIA_Plugin::VIDEO_FRAMES);
    result.push_back(properties);


    return result;
}


DS_NodePtr
RLQPDF_MEDIA_Plugin::createVideoReader(QVariantMap* properties)
{
    RLP_LOG_DEBUG("")

    DS_NodePtr node(new RLQPDF_MEDIA_VideoReader(properties));

    return node;
}


bool
RLQPDF_MEDIA_Plugin::writePdf(GUI_ItemBase* widget, QString path, int width, int height)
{
    RLP_LOG_DEBUG(path)

    bool result = false;

    QPrinter printer(QPrinter::ScreenResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(path);
    printer.setPageSize(QPageSize(QSize(width, height)));

    QPainter p(&printer);
    GUI_QPainter qp(&p);
    
    QVariant v;
    v.setValue(&printer);

    widget->setMetadata("printer", v);
    widget->paint(&qp);

    RLP_LOG_DEBUG("OK")

    return result;
}


//


RLQPDF_MEDIA_ReadTask::RLQPDF_MEDIA_ReadTask(RLQPDF_MEDIA_VideoReader* reader,
                                           DS_TimePtr timeInfo,
                                           DS_FrameBufferPtr destFrame) :
    _reader(reader),
    _timeInfo(timeInfo),
    _destFrame(destFrame)
{
}


void
RLQPDF_MEDIA_ReadTask::run()
{
    // std::cout << "RLQPDF_MEDIA_ReadTask::run()" << std::endl;
    //
    //if ((_reader == nullptr) || (_reader == NULL)) {
    //    return;
    //}

    _reader->readVideo(_timeInfo, _destFrame);
}
