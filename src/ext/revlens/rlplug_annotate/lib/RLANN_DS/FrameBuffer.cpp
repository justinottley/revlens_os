//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include "RlpRevlens/DS/Session.h"
#include "RlpRevlens/CNTRL/MainController.h"
#include "RlpExtrevlens/RLANN_DS/FrameBuffer.h"
#include "RlpExtrevlens/RLANN_DS/Track.h"
#include "RlpExtrevlens/RLANN_DS/QImageFrameBuffer.h"

RLP_SETUP_EXT_LOGGER(RLANN_DS, FrameBuffer)

RLANN_DS_FrameBuffer::RLANN_DS_FrameBuffer(QObject* owner,
                                           QVariantMap frameInfo,
                                           RLANN_DS_DrawControllerBase* drawController):
    DS_FrameBuffer(owner, frameInfo),
    _drawController(drawController)
{
}


DS_BufferList
RLANN_DS_FrameBuffer::getAuxBufferList()
{
    long frame = _bufferMetadata.value("draw.frame").value<long>();

    // RLP_LOG_DEBUG(frame << "ghost frames:" << numGhostFrames)

    DS_BufferList bufList;
    bufList.clear();

    CNTRL_MainController* cntrl = _drawController->getMainController();
    DS_SessionPtr session = cntrl->session();

    for (int tx=0; tx != session->numTracks(); ++tx)
    {

        DS_TrackPtr tr = session->getTrackByIndex(tx);
        if ((tr->trackType() == TRACK_TYPE_ANNOTATION) && (tr->isEnabled()))
        {
            RLANN_DS_Track* track = dynamic_cast<RLANN_DS_Track*>(tr.get());
            int numGhostFrames = track->getGhostFrameCount();
            float ghostOpacity = 1.0 / (float)(numGhostFrames + 1.0);

            for (int gx = 1; gx <= numGhostFrames; ++gx)
            {
                float ghostFrameOpacity = 1 - (ghostOpacity * gx);
                if (track->hasAnnotation(frame - gx))
                {
                    RLANN_DS_AnnotationPtr annGhostBuffer = track->getAnnotation(frame - gx);
                    annGhostBuffer->getBufferMetadata()->insert("opacity", ghostFrameOpacity);

                    bufList.push_back(annGhostBuffer);
                }
            }

            if (track->hasAnnotation(frame))
            {
                RLANN_DS_AnnotationPtr annBuffer = track->getAnnotation(frame);
                annBuffer->getBufferMetadata()->insert("opacity", 1.0);
                bufList.push_back(annBuffer);
            }
        }
    }

    if (_drawController->isPaperTraceActive())
    {
        // RLP_LOG_DEBUG("Adding paper trace frame buffer")

        RLANN_DS_QImageFrameBufferPtr ptfbuf = _drawController->getPaperTraceFBuf();
        if ((ptfbuf->getWidth() != _bufferMetadata.value("width").toInt()) || 
            (ptfbuf->getHeight() != _bufferMetadata.value("height").toInt()))
        {
            RLP_LOG_DEBUG("Reallocating Paper Trace Frame Buffer")

            ptfbuf->reallocate(_bufferMetadata);
            ptfbuf->getQImage()->fill(Qt::white);
            ptfbuf->getBufferMetadata()->insert("opacity", 0.70);
        }

        bufList.append(ptfbuf);
    }

    return bufList;
}


bool
RLANN_DS_FrameBuffer::reload()
{
    RLP_LOG_DEBUG("");

    return false;
}