//
//  Copyright (c) 2014 Justin Ottley. All rights reserved.
//

#ifndef EXTREVLENS_RLEXRDS_FRAMEBUFFER_H
#define EXTREVLENS_RLEXRDS_FRAMEBUFFER_H

#include "RlpExtrevlens/RLEXRDS/Global.h"
#include "RlpRevlens/DS/FrameBuffer.h"

#include <OpenEXR/ImfFrameBuffer.h>


class RLEXRDS_FrameBuffer;
typedef std::shared_ptr<RLEXRDS_FrameBuffer> RLEXRDS_FrameBufferPtr;

class EXTREVLENS_RLEXRDS_API RLEXRDS_FrameBuffer : public DS_FrameBuffer {
    
public:
    RLP_DEFINE_LOGGER

    RLEXRDS_FrameBuffer(QObject* owner, QVariantMap bufferInfo) :
        DS_FrameBuffer(owner, bufferInfo)
    {                         
    }
    
    /*
    ~RLEXRDS_FrameBuffer()
    {
        if (_pixels != NULL) {
            delete _pixels;
            _data = NULL;
        }
    }
    */
    
    
    void reallocate();
    void reallocateHalf();
    void reallocateFloat();

    Imf::FrameBuffer& getFrameBuffer() { return _frameBuffer; }
    
    QImage toQImage();


private:
    
    Imf::FrameBuffer _frameBuffer;
};

#endif

