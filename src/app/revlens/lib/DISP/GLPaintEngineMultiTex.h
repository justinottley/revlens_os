
#ifndef REVLENS_DISP_GLPAINTENGINE_MTEXT_H
#define REVLENS_DISP_GLPAINTENGINE_MTEXT_H

#include "RlpRevlens/DISP/Global.h"
#include "RlpRevlens/DISP/GLPaintEngine2D.h"


class REVLENS_DISP_API DISP_GLPaintEngineMultiTex : public DISP_GLPaintEngine2D {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    DISP_GLPaintEngineMultiTex();

    static DISP_GLPaintEngine2D* create();

private:
    QString getFragmentShaderSource();
    void initializeGLInternalTexture() {} // do nothing
    void paintGLInternalSetupTexture(DS_FrameBuffer* fbuf);

protected:
    DISP_GlslShaderBuilder* _shaderBuilder;

};

#endif

