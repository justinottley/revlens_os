
#include "RlpExtrevlens/RLQTGUI/BlendCompositePlugin.h"

RLP_SETUP_LOGGER(extrevlens, RLQTGUI, BlendCompositePlugin)


RLQTGUI_BlendCompositePlugin::RLQTGUI_BlendCompositePlugin():
    DISP_CompositePlugin("Blend")
{
}


void
RLQTGUI_BlendCompositePlugin::setDisplay(DISP_GLView* display)
{
    DISP_CompositePlugin::setDisplay(display);

    _bslider = new RLQTGUI_BlendCompositeSlider(display);
    _bslider->setVisible(false);

    connect(
        display,
        &DISP_GLView::compositeModeChanged,
        this,
        &RLQTGUI_BlendCompositePlugin::onCompositeModeChanged
    );

    connect(
        _bslider,
        &RLQTGUI_BlendCompositeSlider::moving, // really GUI_Slider
        this,
        &RLQTGUI_BlendCompositePlugin::onBlendPosChanged
    );
}


void
RLQTGUI_BlendCompositePlugin::onCompositeModeChanged(QString compositeMode)
{
    if (compositeMode == _name)
    {
        _bslider->setSliderValue(0.5);
        _bslider->update();
        _bslider->setVisible(true);
        _display->setDisplayMetadata("blend.value", 0.5);

    } else
    {
        _bslider->setVisible(false);
    }
}


void
RLQTGUI_BlendCompositePlugin::onBlendPosChanged(float val)
{
    // RLP_LOG_DEBUG(val)

    _display->setDisplayMetadata("blend.value", val);
}


void
RLQTGUI_BlendCompositePlugin::layout(DS_BufferPtr destFrame, QVariantMap* displayMd)
{
    float pos = displayMd->value("blend.value").toFloat();

    // RLP_LOG_DEBUG(pos)

    QVariantMap* bm = destFrame->getBufferMetadata();
    bm->insert("alpha", pos);

    for (int fi=0; fi != destFrame->numAuxBuffers(false /* includePrimary */); ++fi)
    {
        DS_BufferPtr afbuf = destFrame->getBuffer(fi);
        afbuf->getBufferMetadata()->insert("alpha", 1 - pos);
    }
}


/*
void
RLQTGUI_BlendCompositePlugin::readVideo(DS_Node* nodeIn, DS_TimePtr timeInfo, DS_BufferPtr destFrame)
{
    RLP_LOG_DEBUG("")

    QVariantMap* mbufm = destFrame->getBufferMetadata();
    mbufm->insert("composite_mode", _name);


    float blendVal = 0.5;
    if (nodeIn->getPropertiesPtr()->contains("display.blend.value"))
    {
        blendVal = nodeIn->getPropertiesPtr()->value("display.blend.value").toFloat();
    }

    mbufm->insert("alpha", blendVal);

    QList<DS_BufferPtr> buflist;

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
        bm->insert("composite_mode", _name);
        // bm->insert("node.composite", true);
        bm->insert("node.composite.idx", i);
        bm->insert("media.name", node->getProperty<QString>("media.name"));
        bm->insert("graph.uuid", node->graph()->uuid());


        // Mode specific properties
        //
        bm->insert("alpha", 1 - blendVal);

        n->reallocate();

        destFrame->appendAuxBuffer(n);
        buflist.append(n);
    }

    nodeIn->input(0)->readVideo(timeInfo, destFrame, false); // postUpdate);

    for (int i=1; i != nodeIn->inputCount(); ++i)
    {
        nodeIn->input(i)->readVideo(
            timeInfo,
            buflist.at(i - 1),
            false
        );
    }
}
*/