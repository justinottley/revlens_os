
#include "RlpExtrevlens/RLQTGUI/WipeCompositePlugin.h"

RLP_SETUP_LOGGER(extrevlens, RLQTGUI, WipeCompositePlugin)


RLQTGUI_WipeCompositePlugin::RLQTGUI_WipeCompositePlugin():
    DISP_CompositePlugin("Wipe")
{
}


void
RLQTGUI_WipeCompositePlugin::setDisplay(DISP_GLView* display)
{
    DISP_CompositePlugin::setDisplay(display);

    RLP_LOG_DEBUG("")

    _wsplitter = new RLQTGUI_WipeCompositeSplitter(display);
    _wsplitter->setVisible(false);


    connect(
        _wsplitter,
        &RLQTGUI_WipeCompositeSplitter::posChanged,
        this,
        &RLQTGUI_WipeCompositePlugin::onWipePosChanged
    );

    connect(
        display,
        &DISP_GLView::compositeModeChanged,
        this,
        &RLQTGUI_WipeCompositePlugin::onCompositeModeChanged
    );
}


void
RLQTGUI_WipeCompositePlugin::layout(DS_BufferPtr destFrame, QVariantMap* displayMetadata)
{
    QVariantMap* bm = destFrame->getBufferMetadata();
    bm->insert("alpha", 1.0);

    for (int fi=0; fi != destFrame->numAuxBuffers(false /* includePrimary */); ++fi)
    {
        DS_BufferPtr afbuf = destFrame->getBuffer(fi);
        afbuf->getBufferMetadata()->insert("alpha", 1.0);
    }
}


void
RLQTGUI_WipeCompositePlugin::onCompositeModeChanged(QString newMode)
{
    RLP_LOG_DEBUG(newMode)


    if (newMode == _name)
    {
        if (_wsplitter->posPercent() == -1) // not set yet
        {
            _display->setDisplayMetadata("wipe.pos", 0.5);
            _wsplitter->setPosPercent(0.5);
        }
        _wsplitter->setVisible(true);
    } else
    {
        _wsplitter->setVisible(false);
    }
}


void
RLQTGUI_WipeCompositePlugin::onWipePosChanged(float pos)
{
    // RLP_LOG_DEBUG(pos)

    _display->setDisplayMetadata("wipe.pos", pos);
}


void
RLQTGUI_WipeCompositePlugin::paintGLInternalRecursive(DS_BufferPtr buf, QVariantMap* bufferMd, QVariantMap* displayMd, int recDepth)
{
    // RLP_LOG_DEBUG(*displayMd)

    float pos = -1;
    pos = displayMd->value("wipe.pos").toFloat();
    QRect srect = displayMd->value("srect").value<QRect>();

    int xp = srect.x() + (srect.width() * pos) + 1;
    int wp = srect.width() - (srect.width() * pos);

    int nodeCompIdx = bufferMd->value("node.composite.idx").toInt();

    // RLP_LOG_DEBUG("Scissor:" << nodeCompIdx << "->" << xp << srect.y() << wp << srect.height());

    if (nodeCompIdx == 1)
    {
        glScissor(xp, srect.y(), wp, srect.height());
        glClearColor(0.00, 0.00, 0.00, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    } else
    {
        glScissor(srect.x(), srect.y(), srect.width() + 10, srect.height());
    }
}


/*
void
RLQTGUI_WipeCompositePlugin::readVideo(DS_Node* nodeIn, DS_TimePtr timeInfo, DS_BufferPtr destFrame)
{
    RLP_LOG_DEBUG("")

    QVariantMap* mbufm = destFrame->getBufferMetadata();
    mbufm->insert("composite_mode", _name);


    float wipeXPos = 0.5;
    if (nodeIn->getPropertiesPtr()->contains("display.wipe.xpos"))
    {
        wipeXPos = nodeIn->getPropertiesPtr()->value("display.wipe.xpos").toFloat();
    }

    mbufm->insert("display_wipe.xpos", 0);

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

        bm->insert("composite_mode", _name);
        // bm->insert("node.composite", true);
        bm->insert("node.composite.idx", i);

        bm->insert("display_wipe.xpos", wipeXPos);

        bm->insert("media.name", node->getProperty<QString>("media.name"));
        bm->insert("graph.uuid", node->graph()->uuid());
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