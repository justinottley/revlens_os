//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef EXTREVLENS_RLANN_MEDIA_MEDIAPLUGIN_H
#define EXTREVLENS_RLANN_MEDIA_MEDIAPLUGIN_H

#include "RlpExtrevlens/RLANN_MEDIA/Global.h"

#include "RlpRevlens/DS/FrameBuffer.h"
#include "RlpRevlens/MEDIA/Plugin.h"

#include "RlpExtrevlens/RLANN_DS/DrawControllerBase.h"

class RLANN_DS_FrameBuffer;

typedef std::shared_ptr<RLANN_DS_FrameBuffer> RLANN_DS_FrameBufferPtr;

/*
 * Media plugin for getting the Lookahead to create a RLANN_DS_FrameBuffer,
 * which we need to bridge the RLANN_CNTRL_DrawController and the display system
 */
class EXTREVLENS_RLANN_MEDIA_API RLANN_MEDIA_Plugin : public MEDIA_Plugin {
    Q_OBJECT
    
public:
    RLP_DEFINE_LOGGER

    RLANN_MEDIA_Plugin();
    RLANN_MEDIA_Plugin(RLANN_DS_DrawControllerBase* controller);
    
    DS_FrameBufferPtr createFrameBuffer(QObject* owner, QVariantMap frameInfo);
    
public slots:

    QVariantList identifyMedia(QVariantMap mediaInput, DS_Node::NodeDataType dataType=DS_Node::DATA_UNKNOWN);
    DS_NodePtr createVideoReader(QVariantMap* properties);

    void setDrawController(RLANN_DS_DrawControllerBase* controller);

private:
    
    RLANN_DS_DrawControllerBase* _drawController;
};



#endif