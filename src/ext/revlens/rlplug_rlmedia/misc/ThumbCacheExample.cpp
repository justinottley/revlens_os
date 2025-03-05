// // Example of fetching all thumbnails

    // QVariantList allThumbs = cntrl->getAllFrameData("main.thumbnail");
    // RLP_LOG_DEBUG("Got" << allThumbs.size() << "thumbnail frames");
    // ThumbCacheMap* tcm = new ThumbCacheMap();

    // for (int f = 0; f != allThumbs.size(); ++f)
    // {
    //     QVariantMap finfo = allThumbs.at(f).toMap();
    //     qlonglong fnum = finfo.value("frame_num").toLongLong();
    //     QByteArray imgData = finfo.value("data").toByteArray();

    //     QImage i = QImage::fromData(imgData, "jpg");
    //     tcm->insert(fnum, i);
    // }

    // _thumbCache.setLocalData(tcm);

    // Thumb Cache Example

    // if (destFrame->getBufferMetadata()->contains("video.display_immediate") &&
    //     destFrame->getBufferMetadata()->value("video.display_immediate") != (int)DS_FrameBuffer::FRAME_QUALITY_FULL)
    // {

    //     ThumbCacheMap* tcm = _thumbCache.localData();
    //     if (tcm == nullptr)
    //     {
    //         RLP_LOG_ERROR("no thumb cache")
    //     }

    //     if ((tcm != nullptr) && (tcm->contains(frameNum + 1)))
    //     {
    //         QImage i = tcm->value(frameNum + 1);

    //         // RLP_LOG_DEBUG("Found thumb for frame:" << frameNum << i.width() << "x" << i.height())

    //         QVariantMap frameInfo;
    //         frameInfo.insert("width", i.width());
    //         frameInfo.insert("height", i.height());
    //         frameInfo.insert("pixelSize", 1);
    //         frameInfo.insert("channelCount", 4);
    //         frameInfo.insert("byteCount", i.sizeInBytes());
    //         frameInfo.insert("hasAlpha", i.hasAlphaChannel());

    //         MEDIA_Plugin* plugin = getProperty<MEDIA_Plugin*>("video.plugin");
    //         DS_FrameBufferPtr p = plugin->getOrCreateFrameBuffer(this, frameInfo);

    //         // DS_FrameBufferPtr p(new DS_FrameBuffer(this, frameInfo));
    //         // DS_FrameBuffer p(this, frameInfo);

    //         p->reallocate();
    //         p->setColourSpace(DS_FrameBuffer::COLSPACE_NONLINEAR);
    //         p->setChannelOrder(DS_FrameBuffer::CHANNEL_ORDER_BGRA);
    //         p->setGLTextureFormat(GL_RGBA);
    //         p->setGLTextureInternalFormat(GL_RGBA);

    //         memcpy(p->data(), i.bits(), p->getDataBytes());
    //         // if ((int)i.sizeInBytes() == (int)destFrame->getDataBytes())
    //         // {
                
    //         //     memcpy(p->data(), i.bits(), p->getDataBytes());
    //         //     RLP_LOG_DEBUG("Copy OK:" << p->getDataBytes());
    //         // } else
    //         // {
    //         //     RLP_LOG_ERROR("copy skipped, image:" << i.sizeInBytes() << "vs" << p->getDataBytes())
    //         // }
            

    //         destFrame->appendAuxBuffer(p);

    //         // RLP_LOG_DEBUG("dest frame aux buffer list size:" << destFrame->numAuxBuffers(false));
            
    //     }

    // }
