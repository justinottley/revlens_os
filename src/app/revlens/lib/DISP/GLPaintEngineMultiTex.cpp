

#include "RlpRevlens/DISP/GLPaintEngineMultiTex.h"

RLP_SETUP_LOGGER(revlens, DISP, GLPaintEngineMultiTex)

DISP_GLPaintEngineMultiTex::DISP_GLPaintEngineMultiTex():
    DISP_GLPaintEngine2D()
{
    DS_FrameBuffer::registerGLTextureName("main");

    _shaderBuilder = new DISP_GlslShaderBuilder();
}


DISP_GLPaintEngine2D*
DISP_GLPaintEngineMultiTex::create()
{
    return new DISP_GLPaintEngineMultiTex();
}


QString
DISP_GLPaintEngineMultiTex::getFragmentShaderSource()
{
    QString result = _shaderBuilder->getShaderCode();
    
    std::cout << result.toStdString().c_str() << std::endl;

    return result;
}


void
DISP_GLPaintEngineMultiTex::paintGLInternalSetupTexture(DS_FrameBuffer* fbuf)
{
    QString textureName = fbuf->getGLTextureName();
    RLP_LOG_DEBUG(textureName)

    DS_GLTextureInfo* gtInfo = DS_FrameBuffer::getGLTextureInfoByName(textureName);
    int textureId = gtInfo->getGLTextureId();
    int textureUnit = gtInfo->getGLTextureUnit();

    if (textureId == -1)
    {
        RLP_LOG_WARN("No framebuffer texture, skipping")
        return;
    }

    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, textureId);


    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // RLP_LOG_DEBUG(currFrame->getWidth() << currFrame->getHeight() << currFrame->getDataBytes())

    // RLP_LOG_DEBUG("GL_RGBA:" << GL_RGBA)
    // RLP_LOG_DEBUG("GL_RGB:" << GL_RGB)
    // RLP_LOG_DEBUG("GL_RGB8:" << GL_RGB8)

    // RLP_LOG_DEBUG("GL_UNSIGNED_BYTE:" << GL_UNSIGNED_BYTE)

    // RLP_LOG_DEBUG("currFrame: F:" << currFrame->getGLTextureFormat() << "IF:" << currFrame->getGLTextureInternalFormat() << "T:" << currFrame->getGLTextureType())
    // RLP_LOG_DEBUG(_frameWidth << _frameHeight)


    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        fbuf->getGLTextureInternalFormat(),
        _frameWidth,
        _frameHeight,
        0,
        fbuf->getGLTextureFormat(),
        fbuf->getGLTextureType(),
        fbuf->data()
    );

    QString shaderTextureVarName = QString("%1_texture").arg(textureName);
    QString shaderTextureChanOrder = QString("%1_chan_order").arg(textureName);

    _program->setUniformValue(shaderTextureVarName.toStdString().c_str(), textureUnit);

    GLint chanOrder = fbuf->getChannelOrder();
    _program->setUniformValue(shaderTextureChanOrder.toStdString().c_str(), chanOrder);

}
