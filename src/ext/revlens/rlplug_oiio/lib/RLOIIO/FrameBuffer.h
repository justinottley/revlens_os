

#ifndef EXTREVLENS_RLOIIO_FRAMEBUFFER_H
#define EXTREVLENS_RLOIIO_FRAMEBUFFER_H

#include "RlpExtrevlens/RLOIIO/Global.h"
#include "RlpRevlens/DS/FrameBuffer.h"

class EXTREVLENS_RLOIIO_API RLOIIO_FrameBuffer : public DS_FrameBuffer {

public:
    RLP_DEFINE_LOGGER

    RLOIIO_FrameBuffer(QObject* owner, QVariantMap bufferInfo) :
        DS_FrameBuffer(owner, bufferInfo)
    {                         
    }
    QImage toQImage();

};

#endif