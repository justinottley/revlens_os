//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include <iostream>


#include "RlpRevlens/DISP/GLPaintEngine2D.h"

#ifdef SCAFFOLD_IOS
#include "RlpRevlens/DISP/GlslShaders2DiOS.h"
#elif SCAFFOLD_WASM
#include "RlpRevlens/DISP/GlslShaders2DiOS.h"
#else
#include "RlpRevlens/DISP/GlslShaders2D.h"
#endif


RLP_SETUP_LOGGER(revlens, DISP, GLPaintEngine2D)

DISP_GLPaintEngine2D::DISP_GLPaintEngine2D():
    DISP_GLPaintEngine("Default2D"),
    _textureId(0),
    _program(nullptr),
    _vshader(nullptr),
    _fshader(nullptr),
    _viewportX(0),
    _viewportY(0),
    _frameWidth(0),
    _frameHeight(0),
    _zoom(1.0),
    _zoomPos(0),
    _pan(),
    _panEndPos(),
    _autoFit(true),
    _saturation(1.0),
    _metadata(nullptr)
{
}


DISP_GLPaintEngine2D*
DISP_GLPaintEngine2D::create()
{
    return new DISP_GLPaintEngine2D();
}


void
DISP_GLPaintEngine2D::initializeGLInternalTexture()
{
    // RLP_LOG_DEBUG("")
    glGenTextures(1, &_textureId);
}


QString
DISP_GLPaintEngine2D::getFragmentShaderSource()
{
    return fragment_shader_2D;
}


void
DISP_GLPaintEngine2D::initializeGLInternal(DS_BufferPtr buf)
{
    // RLP_LOG_DEBUG("");

    // TODO FIXME: this method should really take a DS_FrameBufferPtr

    DS_FrameBuffer* currFrame = dynamic_cast<DS_FrameBuffer* >(buf.get());

    // RLP_LOG_DEBUG("buffer: " << currFrame->getDataBytes())

    if (currFrame->getDataBytes() == 0)
    {
        // RLP_LOG_WARN("No framebuffer, skipping")
        return;
    }

    if (_program == nullptr)
    {

        initializeGLInternalTexture();

        char* glslVersion = (char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
        char* glVer = (char*)glGetString(GL_VERSION);

        RLP_LOG_DEBUG("GLSL version:" << glslVersion << "OpenGL Version:" << glVer)
        
        static const int coords[4][3] = {
            { +1, -1, -1 }, { -1, -1, -1 }, { -1, +1, -1 }, { +1, +1, -1 },
        };

        for (int j = 0; j < 4; ++j) {
            _texCoords.append
                (QVector2D(j == 0 || j == 3, j == 0 || j == 1));
            _vertices.append
                (QVector3D(coords[j][0], coords[j][1],
                            coords[j][2]));
        }


        #define PROGRAM_VERTEX_ATTRIBUTE 0
        #define PROGRAM_TEXCOORD_ATTRIBUTE 1

        _vshader = new QOpenGLShader(QOpenGLShader::Vertex, this);
        _vshader->compileSourceCode(vertex_shader_2D);

        _fshader = new QOpenGLShader(QOpenGLShader::Fragment, this);

        QString result = getFragmentShaderSource();
        _fshader->compileSourceCode(result.toStdString().c_str());

        // std::cout << result.toStdString().c_str() << std::endl;

        RLP_LOG_DEBUG("shader log:");
        RLP_LOG_DEBUG(_fshader->log());

        _program = new QOpenGLShaderProgram(this);
        _program->addShader(_vshader);
        _program->addShader(_fshader);
        _program->bindAttributeLocation("vertex", PROGRAM_VERTEX_ATTRIBUTE);
        _program->bindAttributeLocation("texCoord", PROGRAM_TEXCOORD_ATTRIBUTE);
        _program->link();

    }


    _program->bind();

    // glActiveTexture(GL_TEXTURE0 + textureUnit);
    // glBindTexture(GL_TEXTURE_2D, textureId);

    // RLP_LOG_DEBUG("texture bind : " << _textureId << " " << _frameWidth << " " << _frameHeight);


    // glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    // glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    /*
    _program->setUniformValue("texture", textureUnit); // _textureId); // 0); // currTextureId);

    GLint val = 0;
    _program->setUniformValue("chan_order", val);
    
    GLfloat opacity = 1.0;
    _program->setUniformValue("opacity", opacity);

    GLfloat videoGamma = 0.45;
    _program->setUniformValue("videoGamma", videoGamma);
    */

}


void
DISP_GLPaintEngine2D::paintGLInternalSetupTexture(DS_FrameBuffer* fbuf)
{
    // RLP_LOG_DEBUG("")

    // There's this small detail in the OpenGL 2.1 (probably earlier versions
    // too) spec:
    //
    // (For TexImage3D, TexImage2D and TexImage1D):
    // The values of UNPACK ROW LENGTH and UNPACK ALIGNMENT control the row-to-
    // row spacing in these images in the same manner as DrawPixels.
    //
    // UNPACK_ALIGNMENT has a default value of 4 according to the spec. Which
    // means that it was expecting images to be Aligned to 4-bytes, and making
    // several odd "skew-like effects" in the displayed images. Setting the
    // alignment to 1 byte fixes this problems.
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _textureId);

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

    _program->setUniformValue("texture", 0);
    
    GLint chanOrder = fbuf->getChannelOrder();
    _program->setUniformValue("chan_order", chanOrder);


}


void
DISP_GLPaintEngine2D::paintGLInternalOneBuffer(DS_BufferPtr buf)
{
    DS_FrameBuffer* currFrame = dynamic_cast<DS_FrameBuffer* >(buf.get());


    if (currFrame->getChannelCount() == 0)
    {
        return;
    }

    float relZoom = getRelZoom(currFrame->getWidth(), currFrame->getHeight());
    // RLP_LOG_DEBUG(buf->numAuxBuffers() << currFrame->getWidth() << currFrame->getHeight() << relZoom << getZoom())

    _program->bind();


    _frameWidth = currFrame->getWidth();
    _frameHeight = currFrame->getHeight();

    paintGLInternalSetupTexture(currFrame);

    // glClearColor(0.1, 0.1, 0.1, 1.0);

    if (_autoFit)
    {
        resetZoom();
    }


    QMatrix4x4 m;
    _program->setUniformValue("matrix", m);


    //
    // Image and Viewport dimensions
    //
    
    _program->setUniformValue("viewport", _viewportX, _viewportY);
    _program->setUniformValue("image", _frameWidth, _frameHeight);

    QVariantMap bm = buf->getBufferMetadataSafeCopy();
    QString layoutMode = _metadata->value("composite_mode").toString(); // bm.value("composite_mode").toString();

    float layoutScaleX = 1.0;
    float layoutScaleY = 1.0;
    float layoutPanX = 0.0;
    float layoutPanY = 0.0;

    if (layoutMode == "Tile")
    {
        layoutScaleX = bm.value("layout_scale.x").toFloat();
        layoutScaleY = bm.value("layout_scale.y").toFloat();
        layoutPanX = bm.value("layout_pan.x").toFloat();
        layoutPanY = bm.value("layout_pan.y").toFloat();

    }

    float greyscaleMix = 1 - _saturation;
    int doGreyscale = 0;
    if (greyscaleMix > 0)
    {
        doGreyscale = 1;
    }

    if (bm.contains("greyscale_mix"))
    {
        greyscaleMix = bm.value("greyscale_mix").toFloat();
        doGreyscale = 1;
    }

    _program->setUniformValue("layout_scale", layoutScaleX, layoutScaleY);
    _program->setUniformValue("layout_pan", layoutPanX, layoutPanY);
    
    //
    // Zoom and Pan
    //

    _program->setUniformValue("relZoom", relZoom);
    _program->setUniformValue("zoom", getZoom());

    QPointF panPercent = getPanPercent();

    _program->setUniformValue(
        "pan",
        panPercent.x(), panPercent.y());


    //
    // Image channels and color
    //


    GLfloat opacity = bm.value("opacity").toFloat();
    if (opacity == 0)
        opacity = 1.0;


    GLfloat alpha = bm.value("alpha").toFloat();
    if (alpha == 0)
        alpha = 1.0;

    _program->setUniformValue("alpha", alpha);
    _program->setUniformValue("opacity", opacity);

    GLfloat videoGamma = 0.45;
    _program->setUniformValue("videoGamma", videoGamma);

    _program->setUniformValue("do_grayscale", doGreyscale);
    _program->setUniformValue("greyscale_mix", greyscaleMix);

    GLint colspace_linear = currFrame->getColourSpace();
    _program->setUniformValue("colspace_linear", colspace_linear);

    // RLP_LOG_DEBUG(_frameWidth << " " << _frameHeight);
    // RLP_LOG_DEBUG(_viewport.x << " " << _viewport.y);
    // RLP_LOG_DEBUG((float)_frameWidth / (float)_viewport.x);
    // RLP_LOG_DEBUG((float)_frameHeight / (float)_viewport.y);
    
    _program->enableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);
    _program->enableAttributeArray(PROGRAM_TEXCOORD_ATTRIBUTE);
    _program->setAttributeArray
        (PROGRAM_VERTEX_ATTRIBUTE, _vertices.constData());
    _program->setAttributeArray
        (PROGRAM_TEXCOORD_ATTRIBUTE, _texCoords.constData());

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

}


void
DISP_GLPaintEngine2D::paintGLInternalRecursive(DS_BufferPtr buf, int recDepth)
{
    // RLP_LOG_DEBUG("")

    if (buf->getDataBytes() != 0)
    {

        // RLP_LOG_DEBUG("BUF DATA BYTES IS NOT ZERO, PAINTING!!");


        QVariantMap bm = buf->getBufferMetadataSafeCopy();
        QString compositeMode = _metadata->value("composite_mode").toString();

        bool doPaint = true;
        if (bm.contains("node.composite.idx"))
        {
            int nodeCompIdx = bm.value("node.composite.idx").toInt();
            QString dispEnabledKey = QString("node.composite.%1.enabled").arg(nodeCompIdx);
            // RLP_LOG_DEBUG("Checking if enabled:" << dispEnabledKey)

            if ((_metadata->contains(dispEnabledKey) &&
                (!_metadata->value(dispEnabledKey).toBool())))
            {
                // RLP_LOG_DEBUG("DISABLED:" << dispEnabledKey)
                doPaint = false;
            }
        }


        if (doPaint)
        {

            if (_compositePlugins->contains(compositeMode))
            {
                // RLP_LOG_DEBUG(compositeMode)
                // Dispatch layout to Composite Plugin
                //
                DISP_CompositePlugin* cplugin = _compositePlugins->value(compositeMode);
                cplugin->paintGLInternalRecursive(buf, &bm, _metadata, recDepth);
            } else if (compositeMode != "")
            {
                RLP_LOG_WARN("No plugin available for Composite Mode" << compositeMode)
            }


            paintGLInternalOneBuffer(buf);

        }
    }

    // glEnable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    DS_BufferList blist = buf->getBufferList();
    for (int bi=blist.size(); bi !=0; --bi)
    {
        DS_BufferPtr bb = blist.at(bi - 1);
        paintGLInternalRecursive(bb, recDepth + 1);
    }
}


void
DISP_GLPaintEngine2D::paintGLInternal(DS_BufferPtr buf)
{
    // RLP_LOG_DEBUG(buf->numAuxBuffers());

    glClearColor(0.07, 0.07, 0.07, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // TODO FIXME: NOT AVAILABLE ON WINDOWS?
    // glBlendEquation(GL_FUNC_ADD);

    // There's this small detail in the OpenGL 2.1 (probably earlier versions
    // too) spec:
    //
    // (For TexImage3D, TexImage2D and TexImage1D):
    // The values of UNPACK ROW LENGTH and UNPACK ALIGNMENT control the row-to-
    // row spacing in these images in the same manner as DrawPixels.
    //
    // UNPACK_ALIGNMENT has a default value of 4 according to the spec. Which
    // means that it was expecting images to be Aligned to 4-bytes, and making
    // several odd "skew-like effects" in the displayed images. Setting the
    // alignment to 1 byte fixes this problems.
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    if ((_metadata != nullptr) && 
        (_metadata->contains("composite_mode")))
    {
        // RLP_LOG_DEBUG(*_metadata)

        QString compositeMode = _metadata->value("composite_mode").toString();
        if (!_compositePlugins->contains(compositeMode))
        {
            RLP_LOG_ERROR("No plugin available for Composite Mode" << compositeMode)
            return;
        }

        // Dispatch layout to Composite Plugin
        //
        DISP_CompositePlugin* cplugin = _compositePlugins->value(compositeMode);
        cplugin->layout(buf, _metadata);

    }

    paintGLInternalRecursive(buf, 1);
}


QPointF
DISP_GLPaintEngine2D::mapToImage(QPointF& inPoint)
{
    float imgXPercent = inPoint.x() / (float)_viewportX;
    float imgYPercent = inPoint.y() / (float)_viewportY;
    
    // Don't exactly know why
    imgXPercent *= 2.0;
    imgYPercent *= -2.0;
    
    // RLP_LOG_DEBUG("imgXPercent: " << panXPercent);
    // RLP_LOG_DEBUG("imgYPercent: " << panYPercent);
    
    QPointF result(imgXPercent, imgYPercent);
    
    return result;
}


QPointF
DISP_GLPaintEngine2D::getPanPercent()
{
    // RLP_LOG_DEBUG("_frameWidth: " << _frameWidth << " viewport: " << _viewportX);
    // RLP_LOG_DEBUG("panx: " << _pan.x());
    
    QPointF panResult = _panEndPos + mapToImage(_pan); // + 
    return panResult;
    
}


void
DISP_GLPaintEngine2D::resetPan()
{
    _pan = QPointF(0, 0);
    _panEndPos = QPointF(0, 0);

    emit displayPanEnd(_panEndPos);
}


void
DISP_GLPaintEngine2D::setPanPos(QPointF pan)
{
    _pan = pan;
}


void
DISP_GLPaintEngine2D::setPanEnd()
{
    _panEndPos = getPanPercent();
    _pan = QPointF(0, 0);

    emit displayPanEnd(_panEndPos);
}


void
DISP_GLPaintEngine2D::setZoomPos(int zoomX)
{
    _autoFit = false;
    _zoomPos = ((float)zoomX / (float)_viewportX) * 1.2;
}


float
DISP_GLPaintEngine2D::getZoom()
{
    // RLP_LOG_DEBUG("DISP_GLPaintEngine2D::getZoom(): " << _zoom + _zoomPos);
    // RLP_LOG_DEBUG(_zoomPos)
    return _zoom + _zoomPos;
}


float
DISP_GLPaintEngine2D::getRelZoom(int width, int height)
{
    float zoomx = ((float)(_viewportX)) / (float)width;
    float zoomy = ((float)(_viewportY)) / (float)height;

    float startZoom = std::min(zoomx, zoomy);

    return startZoom;
}


float
DISP_GLPaintEngine2D::getDisplayZoom()
{
    return getRelZoom(_frameWidth, _frameHeight);
}


void
DISP_GLPaintEngine2D::setZoomEnd()
{
    _zoom += _zoomPos;
    _zoomPos = 0;
    
    RLP_LOG_DEBUG("zoom end: " << _zoom);

    emit displayZoomEnd(getDisplayZoom() + getZoom()); // _zoom);
}


void
DISP_GLPaintEngine2D::resizeGLInternal(int width, int height)
{
    // RLP_LOG_DEBUG(width << " " << height)
    setSize(width, height);
}


void
DISP_GLPaintEngine2D::setSize(int width, int height)
{
    // RLP_LOG_DEBUG(width << " " << height)

    _viewportX = (float)width;
    _viewportY = (float)height;

    resetZoom();
}


void
DISP_GLPaintEngine2D::resetZoom()
{
    _zoom = 0;

    emit displayZoomEnd(getDisplayZoom() + getZoom());
}
