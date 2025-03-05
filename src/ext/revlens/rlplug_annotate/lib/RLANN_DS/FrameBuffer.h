//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef EXTREVLENS_RLANN_DS_FRAMEBUFFER_H
#define EXTREVLENS_RLANN_DS_FRAMEBUFFER_H

#include "RlpExtrevlens/RLANN_DS/Global.h"

#include "RlpRevlens/DS/FrameBuffer.h"
#include "RlpRevlens/CNTRL/MainController.h"

#include "RlpExtrevlens/RLANN_DS/DrawControllerBase.h"

class RLANN_DS_FrameBuffer;

typedef std::shared_ptr<RLANN_DS_FrameBuffer> RLANN_DS_FrameBufferPtr;
/*
 * To achieve a live bridge between the RLANN_CNTRL_DrawController and the display system
 */
class EXTREVLENS_RLANN_DS_API RLANN_DS_FrameBuffer : public DS_FrameBuffer {
    
public:
    RLP_DEFINE_LOGGER


public:
    
    RLANN_DS_FrameBuffer(QObject* owner,
                         QVariantMap frameInfo,
                         RLANN_DS_DrawControllerBase* drawController);
    
    virtual DS_BufferList getAuxBufferList();
    virtual bool reload();

private:
    
    RLANN_DS_DrawControllerBase* _drawController;
};

#endif
