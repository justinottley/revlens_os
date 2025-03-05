

#include "RlpRevlens/MEDIA/Plugin.h"
#include "RlpRevlens/MEDIA/CompositeNode.h"

RLP_SETUP_LOGGER(revlens, MEDIA, CompositeNode)

MEDIA_CompositeNode::MEDIA_CompositeNode(QVariantMap* properties, DS_NodePtr nodeIn):
    DS_Node("MEDIA_Composite", properties)
{
    QVariant nWrapper;
    nWrapper.setValue(this);
    properties->insert("graph.node.composite", nWrapper);

    _inputs.clear();
    _inputs.push_back(nodeIn);
}


void
MEDIA_CompositeNode::addInput(DS_NodePtr nodeIn)
{
    RLP_LOG_DEBUG("")

    _inputs.push_back(nodeIn);

    if (_inputs.size() == 1)
    {
        return;
    }


    // recalculate total byteCount / mem required to read all media
    //
    QVariantMap frameInfo = nodeIn->getProperty<QVariantMap>("video.frame_info.one");
    int nByteCount = frameInfo.value("byteCount").value<int>();

    QVariantMap gFrameInfo = getProperty<QVariantMap>("video.frame_info.one");
    int gByteCount = gFrameInfo.value("byteCount").value<int>();

    gByteCount += nByteCount;
    gFrameInfo.insert("byteCount", gByteCount);
    gFrameInfo.insert("media_count", (int)_inputs.size());

    _properties->insert("video.frame_info.one", gFrameInfo);

    QString mediaName = _properties->value("media.name").toString();
    _properties->insert("media.name.orig", mediaName);

    mediaName += " | ";
    mediaName += nodeIn->getProperty<QString>("media.name");

    _properties->insert("media.name", mediaName);

    // need to insert session.frame for thumbnail.....
    nodeIn->getPropertiesPtr()->insert("session.frame", getProperty<qlonglong>("session.frame"));

    QVariant nodeWrapper;
    nodeWrapper.setValue(nodeIn);

    qlonglong midFrame = int(nodeIn->getProperty<qlonglong>("media.frame_count") / 2);
    QVariantList frameList;
    frameList.append(midFrame);

    QVariantMap nameInfo;
    nameInfo.insert("data_type", "media_input");
    nameInfo.insert("value", mediaName);
    nameInfo.insert("node", nodeWrapper);
    nameInfo.insert("frame_list", frameList);

    emitDataReadyToGraph(nameInfo);
}


void
MEDIA_CompositeNode::readVideo(DS_TimePtr timeInfo, DS_BufferPtr destFrame, bool postUpdate)
{
    int inputCount = _inputs.size();

    // RLP_LOG_DEBUG(timeInfo->videoFrame() << " inputs: " << inputCount)

    if (inputCount > 1)
    {
        // RLP_LOG_DEBUG("comp readvideo")

        QString compositeMode = "Tile";
        if (_properties->contains("display.composite_mode"))
        {
            compositeMode = _properties->value("display.composite_mode").toString();
        }

        float wipeXPos = 0.5;
        if (_properties->contains("display.wipe.xpos"))
        {
            wipeXPos = _properties->value("display.wipe.xpos").toFloat();
        }

        float blendVal = 0.5;
        if (_properties->contains("display.blend.value"))
        {
            blendVal = _properties->value("display.blend.value").toFloat();
        }
        // Override owner of buffer so when event is posted after read we can intercept,
        // remap back to source space, and then resend event to source owner
        //

        QVariantMap* mbufm = destFrame->getBufferMetadata();

        // NOTE: owner redirection disabled. Not performant on macOS (?)
        //
        // QVariant ownerWrapper;
        // ownerWrapper.setValue(destFrame->getOwner());
        // mbufm->insert("node.composite.buffer_src_owner", ownerWrapper);
        // destFrame->setOwner(this);

        mbufm->insert("graph.uuid", graph()->uuid());
        mbufm->insert("node.composite.idx", 0);

        if (compositeMode == "Tile")
        {
            mbufm->insert("layout_scale.x", 0.5);
            mbufm->insert("layout_scale.y", 0.5);
            mbufm->insert("layout_pan.x", -0.5);
            mbufm->insert("layout_pan.y", 0.0);

            mbufm->insert("display_pan.x", 0);

        } else if (compositeMode == "Wipe")
        {
            // TODO FIXME: bug in logic here? I came back to this - what is the bug?
            // is it that the wipe pos is set to the actual value on the next framebuffer?
            mbufm->insert("display_wipe.xpos", 0);

        } else if (compositeMode == "Blend")
        {
            mbufm->insert("alpha", blendVal);
        }


        mbufm->insert("composite_mode", compositeMode);
        mbufm->insert("media.name", _properties->value("media.name.orig"));

        QVariant destFrameWrapper;
        destFrameWrapper.setValue(destFrame);
        mbufm->insert("node.composite.buffer_main", destFrameWrapper);

        QList<DS_BufferPtr> buflist;

        for (int i=1; i != _inputs.size(); ++i)
        {
            DS_NodePtr node = _inputs.at(i);
            MEDIA_Plugin* plugin = node->getProperty<MEDIA_Plugin*>("video.plugin");
            QVariantMap frameInfo = node->getProperty<QVariantMap>("video.frame_info.one");

            DS_FrameBufferPtr n = plugin->getOrCreateFrameBuffer(this, frameInfo);
            n->reallocate();
            n->setValid(true);
            n->setReady(false);

            QVariantMap* bm = n->getBufferMetadata();

            bm->insert("composite_mode", compositeMode);
            bm->insert("node.composite", true);
            bm->insert("node.composite.idx", i);
            bm->insert("node.composite.buffer_main", destFrameWrapper);

            // NOTE: only 2 framebuffers supported at the moment
            //
            if (compositeMode == "Tile")
            {
                bm->insert("layout_scale.x", 0.5);
                bm->insert("layout_scale.y", 0.5);
                bm->insert("layout_pan.x", 0.5);
                bm->insert("layout_pan.y", 0.0);

                bm->insert("display_pan.x", 0.5);

            } else if (compositeMode == "Wipe")
            {
                bm->insert("display_wipe.xpos", wipeXPos);

            } else if (compositeMode == "Blend")
            {
                bm->insert("alpha", 1 - blendVal);
            }

            bm->insert("media.name", node->getProperty<QString>("media.name"));
            bm->insert("graph.uuid", graph()->uuid());
            n->reallocate();

            destFrame->appendAuxBuffer(n);
            buflist.append(n);
        }

        // RLP_LOG_DEBUG("READ MAIN FRAME " << destFrame->getOwner())

        _inputs[0]->readVideo(timeInfo, destFrame, false); // postUpdate);

        for (int i=1; i != _inputs.size(); ++i)
        {
            _inputs[i]->readVideo(
                timeInfo,
                buflist.at(i - 1),
                false
            );
        }

        if (postUpdate)
        {

            // RLP_LOG_DEBUG("POST UPDATE "  << timeInfo->mediaVideoFrame() << destBuffer->getOwner())

            QApplication::postEvent(destFrame->getOwner(),
                                    new MEDIA_ReadEvent(timeInfo,
                                                        getProperty<int>("index"),
                                                        destFrame,
                                                        MEDIA_FrameReadEventType));
        }
    }
    else
    {
        _inputs[0]->readVideo(timeInfo, destFrame, postUpdate);

    }

}


void
MEDIA_CompositeNode::readAudio(DS_TimePtr timeInfo, DS_BufferPtr destBuffer)
{
    return _inputs[0]->readAudio(timeInfo, destBuffer);
}


void
MEDIA_CompositeNode::scheduleReadTask(
    DS_TimePtr timeInfo,
    DS_BufferPtr destFrame,
    DS_Node::NodeDataType dataType,
    bool optional)
{
    QRunnable* task = new MEDIA_CompositeReadTask(this, timeInfo, destFrame);
    QThreadPool* tpool = _properties->value("video.plugin").
                                      value<MEDIA_Plugin *>()->
                                      getReadThreadPool(dataType);

    if (optional)
    {
        tpool->tryStart(task);
    } else
    {
        tpool->start(task);
    }
}



bool
MEDIA_CompositeNode::event(QEvent* event)
{
    // RLP_LOG_DEBUG("")

    // NOTE: This isn't used anymore. Turns out on macOS, owner redirection of the framebuffer
    // is too slow and the composite can't run at playback rate. So the CompositeNode now just
    // sequentially pulls on all inputs in read. Ideally we would asyncronously read each input
    // in separate threads and then post to the lookahead when all are ready. Revisit?
    //
    /// TODO FIXME: remove this?
    //

    if (event->type() == MEDIA_FrameReadEventType) {

        MEDIA_ReadEvent* fre = dynamic_cast<MEDIA_ReadEvent *>(event);

        DS_BufferPtr fbuf = fre->getFrameBuffer();
        fbuf->setReady(true);

        QVariantMap* mdata = fbuf->getBufferMetadata();
        DS_BufferPtr mainBuf = mdata->value("node.composite.buffer_main").
                                      value<DS_BufferPtr>();


        // count number of auxiliar composite buffers
        int numCompositeBuffers = 0;
        int numReady = 0;


        for (int bi=0; bi != mainBuf->numAuxBuffers(false); ++bi)
        {
            QVariantMap* auxdata = mainBuf->getAuxBuffer(bi)->getBufferMetadata();
            if (!auxdata->contains("node.composite"))
            {
                // RLP_LOG_DEBUG("Skipping buffer, node.composite key not found");
                continue;
            }

            numCompositeBuffers++;

            if (mainBuf->getAuxBuffer(bi)->isReady())
            {
                numReady++;
            }
        }

        // RLP_LOG_DEBUG("num aux composite buffers: " << numCompositeBuffers << " num ready: " << numReady)


        if ((numReady == numCompositeBuffers) &&
            (mainBuf->isReady()))
        {
            // RLP_LOG_INFO("ALL BUFFERS READY")

            QObject* srcOwner = mainBuf->getBufferMetadata()->value("node.composite.buffer_src_owner").
                                       value<QObject*>();

            QApplication::postEvent(
                srcOwner,
                new MEDIA_ReadEvent(
                    fre->getTimeInfo(),
                    fre->getNodeIndex(),
                    fre->getFrameBuffer(),
                    MEDIA_FrameReadEventType));

            return true;

        }


        return true;
    }

    return false;
}


MEDIA_CompositeReadTask::MEDIA_CompositeReadTask(
    MEDIA_CompositeNode* reader,
    DS_TimePtr timeInfo,
    DS_BufferPtr destFrame
):
    _reader(reader),
    _timeInfo(timeInfo),
    _destFrame(destFrame)
{
}


void
MEDIA_CompositeReadTask::run()
{
    _reader->readVideo(_timeInfo, _destFrame);
}