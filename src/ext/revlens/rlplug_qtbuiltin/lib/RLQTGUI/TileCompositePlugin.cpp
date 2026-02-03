
#include "RlpExtrevlens/RLQTGUI/TileCompositePlugin.h"

RLP_SETUP_LOGGER(extrevlens, RLQTGUI, CompositeImageWindow)

RLP_SETUP_LOGGER(extrevlens, RLQTGUI, TileCompositePlugin)


RLQTGUI_CompositeImageWindow::RLQTGUI_CompositeImageWindow(
    GUI_ItemBase* parent,
    QString displayUuidStr,
    int fbufWidth,
    int fbufHeight,
    QVariantMap* metadata):
        GUI_ItemBase(parent),
        _displayUuidStr(displayUuidStr),
        _fbufWidth(fbufWidth),
        _fbufHeight(fbufHeight),
        _windowWidth(0),
        _windowHeight(0),
        _displayZoom(1.0),
        _inHover(false),
        _enabled(true),
        _metadata(metadata)
{
    setAcceptHoverEvents(true);
}


void
RLQTGUI_CompositeImageWindow::hoverEnterEvent(QHoverEvent* event)
{
    // RLP_LOG_DEBUG("")

    _inHover = true;
    update();
}


void
RLQTGUI_CompositeImageWindow::hoverLeaveEvent(QHoverEvent* event)
{
    // RLP_LOG_DEBUG("")

    _inHover = false;
    update();
}


void
RLQTGUI_CompositeImageWindow::refresh(int nwidth, int nheight, float displayZoom, QVariantMap* fbufMetadata)
{
    //
    // This is assuming a x-oriented layout.
    // The algorithm here is a reproduction of what happens in opengl
    //
    qreal layoutScaleX = fbufMetadata->value("layout_scale.x").value<qreal>();
    qreal layoutScaleY = fbufMetadata->value("layout_scale.y").value<qreal>();

    qreal displayPanX = fbufMetadata->value("display_pan.x").value<qreal>();

    // RLP_LOG_DEBUG(nwidth << nheight << displayZoom << layoutScaleX)
    if ((layoutScaleX) == 1)
    {
        return;
    }


    _windowWidth = nwidth * layoutScaleX;
    _windowHeight = nheight;

    // RLP_LOG_DEBUG("")
    // RLP_LOG_DEBUG("fbuf:" << _fbufWidth << _fbufHeight)
    // RLP_LOG_DEBUG(nwidth << nheight << displayZoom)
    // RLP_LOG_DEBUG("layout:" << layoutScaleX << layoutScaleY)
    // RLP_LOG_DEBUG("pan:" << displayPanX)   
    // RLP_LOG_DEBUG("window:" << _windowWidth << _windowHeight)

    _displayZoom = displayZoom; // not used; broken for composites
    
    qreal maxHeight = nheight * layoutScaleY;
    qreal imageScale = (float)_windowWidth / (float)_fbufWidth;
    
    if (_fbufWidth > _windowWidth)
    {
        _nDestWidth = _windowWidth;
    }

    _nDestHeight = _fbufHeight * imageScale;

    if (_nDestHeight > maxHeight)
    {
        _nDestHeight = maxHeight;

        // rescale width to new height
        //
        imageScale = (float)_nDestHeight / (float)_fbufHeight;
        _nDestWidth = _fbufWidth * imageScale;
    }

    qreal xOffset = nwidth * displayPanX;

    _nLeft = xOffset + ((_windowWidth - _nDestWidth) / 2.0);
    _nTop = ((float)(_windowHeight - _nDestHeight)) / 2.0;

    // RLP_LOG_DEBUG("pos:" << _nLeft << _nTop)

    setWidth(_nDestWidth);
    setHeight(_nDestHeight);
    setPos(_nLeft, _nTop);

    // RLP_LOG_DEBUG("layoutscale:" << layoutScaleX << "display_pan:" << displayPanX << "displayzoom:" << displayZoom)
    // RLP_LOG_DEBUG("nLeft:" << _nLeft << "xOffset:" << xOffset << "imageScale:" << imageScale)
    // RLP_LOG_DEBUG("Window incoming:" << width << height)
    // RLP_LOG_DEBUG("windowWidth:" << _windowWidth << _windowHeight << "image:" << _width << _height << "pos:" << _nLeft << _nTop << displayZoom)
    // RLP_LOG_DEBUG("nDest:" << _nDestWidth << _nDestHeight)

}



QRectF
RLQTGUI_CompositeImageWindow::boundingRect() const
{
    return QRectF(2, 2, width() - 4, height() - 4);
}


void
RLQTGUI_CompositeImageWindow::paint(GUI_Painter* painter)
{
    // RLP_LOG_DEBUG(_enabled << _inHover)

    if (!_enabled)
    {
        return;
    }

    if (!_inHover)
    {
        return;
    }

    QPen p(QColor(120, 120, 120));
    p.setWidth(5);

    QBrush b(Qt::transparent);

    painter->setPen(p);
    painter->setBrush(b);
    painter->drawRect(boundingRect());

    // drop shadow
    painter->setPen(QColor(20, 20, 20));
    painter->drawText(
        21,
        height() - 9,
        _metadata->value("media.name").toString()
    );

    painter->setPen(QColor(230, 230, 230));
    painter->drawText(
        20,
        height() - 10,
        _metadata->value("media.name").toString()
    );
}


// ----------------------


RLQTGUI_TileCompositePlugin::RLQTGUI_TileCompositePlugin():
    DISP_CompositePlugin("Tile")
{
    connect(
        this,
        &RLQTGUI_TileCompositePlugin::imageWindowNeeded,
        this,
        &RLQTGUI_TileCompositePlugin::onImageWindowNeeded
    );
}


void
RLQTGUI_TileCompositePlugin::recalculateImageRects(const QVariantMap& paintMetadata, DS_FrameBufferPtr fbuf)
{
    // DS_BufferPtr fbuf,
    // qreal wx,
    // qreal wy,
    // qreal zoom)

    // RLP_LOG_DEBUG(paintMetadata)

    qreal wx = paintMetadata.value("window.width").value<qreal>();
    qreal wy = paintMetadata.value("window.height").value<qreal>();
    qreal zoom = paintMetadata.value("display.zoom").value<qreal>();

    // DS_FrameBufferPtr fbuf = paintMetadata.value("video.framebuffer").value<DS_FrameBufferPtr>();
    QVariantMap* bm = fbuf->getBufferMetadata();

    // RLP_LOG_DEBUG(*bm)
    int compositeIdx = bm->value("node.composite.idx").value<int>();

    // RLP_LOG_DEBUG(compositeIdx)


    if (!_imgMap.contains(compositeIdx))
    {
        // Cannot construct CompositeImageWindow widget here since
        // we're potentially in the render thread. Emit a signal instead for slot
        // connection in the main thread
        
        // QVariantMap imw;
        // imw.insert("width", bm->value("width").value<int>());
        // imw.insert("height", bm->value("height").value<int>());
        // imw.insert("node.composite.idx", bm->value("node.composite.idx").value<int>());
        // imw.insert("display", )


        GUI_ItemBase* parent = paintMetadata.value("display.obj").value<GUI_ItemBase*>();
        emit imageWindowNeeded(parent, bm);
        return;
    }

    _imgMap.value(compositeIdx)->setEnabled(_enabled);
    _imgMap.value(compositeIdx)->refresh(wx, wy, zoom, bm);

    for (int fi=0; fi != fbuf->numAuxBuffers(/* includePrimary= */ false); ++fi)
    {
        DS_BufferPtr afbuf = fbuf->getBuffer(fi);
        // recalculateImageRects(paintMetadata, afbuf); // afbuf, wx, wy, zoom);
        // recalculateImageRects(paintMetadata);
    }

}


void
RLQTGUI_TileCompositePlugin::onImageWindowNeeded(GUI_ItemBase* parent, QVariantMap* bm)
{
    RLP_LOG_DEBUG(parent)

    int fbufWidth = bm->value("width").value<int>();
    int fbufHeight = bm->value("height").value<int>();
    int compositeIdx = bm->value("node.composite.idx").value<int>();


    RLQTGUI_CompositeImageWindow* w = new RLQTGUI_CompositeImageWindow(parent, "", fbufWidth, fbufHeight, bm);
    w->setEnabled(_enabled);

    _imgMap.insert(compositeIdx, w);
}


bool
RLQTGUI_TileCompositePlugin::isBufferEnabled(int bufferIdx, QVariantMap* displayMetadata)
{
    QString dispEnabledKey = QString("node.composite.%1.enabled").arg(bufferIdx);
    if (displayMetadata->contains(dispEnabledKey))
    {
        if (!displayMetadata->value(dispEnabledKey).toBool())
        {
            return false;
        }
    }

    return true;
}


void
RLQTGUI_TileCompositePlugin::layout(DS_BufferPtr destFrame, QVariantMap* displayMetadata)
{
    int inputCount = destFrame->numAuxBuffers();
    // RLP_LOG_DEBUG(inputCount)
    // float posScale = 1.0 / (float)inputCount;

    // check whether any of the inputs are disabled in this display
    //
    int enabledCount = 0;
    for (int dx=0; dx != inputCount; ++dx)
    {
        if (!isBufferEnabled(dx, displayMetadata))
        {
            continue;
        }

        enabledCount++;
    }

    // RLP_LOG_DEBUG("enabledCount:" << enabledCount)


    QVariantMap* mbufm = destFrame->getBufferMetadata();
    mbufm->insert("alpha", 1.0);

    // TODO FIXME: "Horizontal" layout, "Vertical" layout
    // TODO FIXME: doesn't respect which buffers are enabled

    if (enabledCount == 2)
    {
        // HACK: only one of the buffers is enabled, just reset for all
        // FIX THIS, reset only enabled buffer

        // RLP_LOG_DEBUG("Single enabled")

        mbufm->insert("layout_scale.x", 1.0);
        mbufm->insert("layout_scale.y", 1.0);
        mbufm->insert("layout_pan.x", 0.0);
        mbufm->insert("layout_pan.y", 0.0);
        
        for (int dx=0; dx != inputCount - 1; ++dx)
        {
            if (isBufferEnabled(dx, displayMetadata))
            {
                // RLP_LOG_DEBUG("found buffer at idx" << dx)
                QVariantMap* bm = destFrame->getBuffer(dx)->getBufferMetadata();

                bm->insert("layout_scale.x", 1.0);
                bm->insert("layout_scale.y", 1.0);
                bm->insert("layout_pan.x", 0.0);
                bm->insert("layout_pan.y", 0.0);
                
                bm->insert("alpha", 1.0);
                // continue;
            }
        }

        /*
        if (inputCount > 2)
        {
            QVariantMap* bm = destFrame->getBuffer(1)->getBufferMetadata();

            bm->insert("layout_scale.x", 1.0);
            bm->insert("layout_scale.y", 1.0);
            bm->insert("layout_pan.x", 0.0);
            bm->insert("layout_pan.y", 0.0);

        }

        else if (inputCount > 3)
        {
            QVariantMap* bmm = destFrame->getBuffer(2)->getBufferMetadata();

            bmm->insert("layout_scale.x", 1.0);
            bmm->insert("layout_scale.y", 1.0);
            bmm->insert("layout_pan.x", 0.0);
            bmm->insert("layout_pan.y", 0.0);

            bmm->insert("display_pan.x", 0.0);

        }
        */

    }
    else if (enabledCount == 3) // HACK: annotation system adds buffer at 0, making 3
    {
        // Main (first) FrameBuffer
        
        if (inputCount == 4)
        {
            int firstIdx = 1;
            int secondIdx = 2;

            QVariantMap* bm1 = destFrame->getBuffer(firstIdx)->getBufferMetadata();

            bm1->insert("layout_scale.x", 0.5);
            bm1->insert("layout_scale.y", 0.5);
            bm1->insert("layout_pan.x", -0.5);
            bm1->insert("layout_pan.y", 0.0);

            bm1->insert("display_pan.x", 0);

            bm1->insert("alpha", 1.0);

            // TODO FIXME HACK
            // the annotation system adds a buffer at position 0
            QVariantMap* bm = destFrame->getBuffer(secondIdx)->getBufferMetadata();

            // Mode specific properties
            //
            bm->insert("layout_scale.x", 0.5);
            bm->insert("layout_scale.y", 0.5);
            bm->insert("layout_pan.x", 0.5);
            bm->insert("layout_pan.y", 0.0);

            bm->insert("display_pan.x", 0.0);

            bm->insert("alpha", 1.0);

        } else
        {
            mbufm->insert("layout_scale.x", 0.5);
            mbufm->insert("layout_scale.y", 0.5);
            mbufm->insert("layout_pan.x", -0.5);
            mbufm->insert("layout_pan.y", 0.0);

            mbufm->insert("display_pan.x", 0);

            // TODO FIXME HACK
            // the annotation system adds a buffer at position 0
            QVariantMap* bm = destFrame->getBuffer(1)->getBufferMetadata();

            // Mode specific properties
            //
            bm->insert("layout_scale.x", 0.5);
            bm->insert("layout_scale.y", 0.5);
            bm->insert("layout_pan.x", 0.5);
            bm->insert("layout_pan.y", 0.0);

            bm->insert("display_pan.x", 0.0);

            bm->insert("alpha", 1.0);
        }
        
    }

    else if (enabledCount == 4)
    {
 
        // Main (first) FrameBuffer
        mbufm->insert("layout_scale.x", 0.4);
        mbufm->insert("layout_scale.y", 0.4);
        mbufm->insert("layout_pan.x", -0.5);
        mbufm->insert("layout_pan.y", 0.5);

        mbufm->insert("display_pan.x", 0);

        // TODO FIXME HACK
        // the annotation system adds a buffer at position 0
        QVariantMap* bm = destFrame->getBuffer(1)->getBufferMetadata();

        // Mode specific properties
        //
        bm->insert("layout_scale.x", 0.4);
        bm->insert("layout_scale.y", 0.4);
        bm->insert("layout_pan.x", 0.5);
        bm->insert("layout_pan.y", 0.5);

        bm->insert("display_pan.x", 0.5);
        
        bm->insert("alpha", 1.0);

        QVariantMap* bmm = destFrame->getBuffer(2)->getBufferMetadata();

        // Mode specific properties
        //
        bmm->insert("layout_scale.x", 0.6);
        bmm->insert("layout_scale.y", 0.6);
        bmm->insert("layout_pan.x", 0.0);
        bmm->insert("layout_pan.y", -0.5);

        bmm->insert("display_pan.x", 0.0);
        
        bmm->insert("alpha", 1.0);
    } 
}


/*

void
RLQTGUI_TileCompositePlugin::readVideo(DS_Node* nodeIn, DS_TimePtr timeInfo, DS_BufferPtr destFrame)
{
    RLP_LOG_DEBUG("")

    QVariantMap* mbufm = destFrame->getBufferMetadata();
    mbufm->insert("composite_mode", _name);


    // QList<DS_BufferPtr> buflist;

    for (int i=1; i != nodeIn->inputCount(); ++i)
    {
        DS_NodePtr node = nodeIn->input(i);
        MEDIA_Plugin* plugin = node->getProperty<MEDIA_Plugin*>("video.plugin");
        QVariantMap frameInfo = node->getProperty<QVariantMap>("video.frame_info.one");

        DS_FrameBufferPtr n = plugin->getOrCreateFrameBuffer(this, frameInfo);
        n->reallocate();
        n->setValid(true);
        n->setReady(false);

        QVariantMap* bm = n->getBufferMetadata();

        // Standard properties
        //
        bm->insert("composite_mode", _name); // compositeMode);
        // bm->insert("node.composite", true);
        bm->insert("node.composite.idx", i);
        bm->insert("media.name", node->getProperty<QString>("media.name"));
        bm->insert("graph.uuid", node->graph()->uuid());

        n->reallocate();

        destFrame->appendAuxBuffer(n);
        // buflist.append(n);
    }


    // Add metadata for layout
    //
    // layout(nodeIn, destFrame);


    nodeIn->input(0)->readVideo(timeInfo, destFrame, false); // postUpdate);

    for (int i=1; i != nodeIn->inputCount(); ++i)
    {
        nodeIn->input(i)->readVideo(
            timeInfo,
            destFrame->getBuffer(i), // (TODO FIXME, hack) the annotation system adds a buffer at position 0 
            false
        );
    }
}

*/



void
RLQTGUI_TileCompositePlugin::paintGLPostDraw(GUI_Painter* painter, const QVariantMap& metadata)
{
    // TODO FIXME: disabled for now, only partially working again (first media only??)
    return;

    // RLP_LOG_DEBUG("")

    DS_FrameBufferPtr fbuf = metadata.value("video.framebuffer").value<DS_FrameBufferPtr>();
    recalculateImageRects(metadata, fbuf); // fbuf, wx, wy, zoom);

}