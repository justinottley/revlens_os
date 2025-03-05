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
    _saturation(1.0)
{
    _metadata.clear();
}


DISP_GLPaintEngine2D*
DISP_GLPaintEngine2D::create()
{
    return new DISP_GLPaintEngine2D();
}


int
DISP_GLPaintEngine2D::getCurrentGlTextureId()
{
    return 0;
}


void
DISP_GLPaintEngine2D::initializeGLInternal(DS_BufferPtr buf)
{
    // RLP_LOG_DEBUG("");

    DS_FrameBuffer* currFrame = dynamic_cast<DS_FrameBuffer* >(buf.get());
    
    // RLP_LOG_DEBUG("buffer: " << currFrame->getDataBytes())

    if (currFrame->getDataBytes() == 0)
    {
        RLP_LOG_WARN("No framebuffer, skipping")
        return;
    }

    if (_program == nullptr)
    {
    
        glGenTextures(1, &_textureId);

        char* glslVersion = (char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
        char* glVer = (char*)glGetString(GL_VERSION);

        RLP_LOG_DEBUG("Texture Created - GLSL version:" << glslVersion << "OpenGL Version:" << glVer)
        
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
        _fshader->compileSourceCode(fragment_shader_2D);
        
        RLP_LOG_DEBUG("shader log:");
        RLP_LOG_DEBUG(_fshader->log());
        
        _program = new QOpenGLShaderProgram(this);
        _program->addShader(_vshader);
        _program->addShader(_fshader);
        _program->bindAttributeLocation("vertex", PROGRAM_VERTEX_ATTRIBUTE);
        _program->bindAttributeLocation("texCoord", PROGRAM_TEXCOORD_ATTRIBUTE);
        _program->link();

    }


    glBindTexture(GL_TEXTURE_2D, _textureId);
    
    // RLP_LOG_DEBUG("texture bind : " << _textureId << " " << _frameWidth << " " << _frameHeight);


    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


    _program->bind();


    _program->setUniformValue("texture", 0); // _textureId); // 0); // currTextureId);
    
    GLint val = 0;
    _program->setUniformValue("chan_order", val);
    
    GLfloat opacity = 1.0;
    _program->setUniformValue("opacity", opacity);

    GLfloat videoGamma = 0.45;
    _program->setUniformValue("videoGamma", videoGamma);

    
}


void
DISP_GLPaintEngine2D::paintGLInternalOneBuffer(DS_BufferPtr buf)
{
    DS_FrameBuffer* currFrame = dynamic_cast<DS_FrameBuffer* >(buf.get());

    // RLP_LOG_DEBUG(buf->numAuxBuffers() << currFrame->getWidth() << currFrame->getHeight() << currFrame->getChannelCount())

    if (currFrame->getChannelCount() == 0)
    {
        return;
    }
    // glClearColor(0.1, 0.1, 0.1, 1.0);

    glBindTexture(GL_TEXTURE_2D, _textureId); // currTextureId);

    // RLP_LOG_DEBUG(currFrame->getWidth() << currFrame->getHeight() << currFrame->getDataBytes())

    // RLP_LOG_DEBUG("GL_RGBA:" << GL_RGBA)
    // RLP_LOG_DEBUG("GL_RGB:" << GL_RGB)
    // RLP_LOG_DEBUG("GL_RGB8:" << GL_RGB8)

    // RLP_LOG_DEBUG("GL_UNSIGNED_BYTE:" << GL_UNSIGNED_BYTE)

    // RLP_LOG_DEBUG("currFrame: F:" << currFrame->getGLTextureFormat() << "IF:" << currFrame->getGLTextureInternalFormat() << "T:" << currFrame->getGLTextureType())
    // RLP_LOG_DEBUG(_frameWidth << _frameHeight)

    _frameWidth = currFrame->getWidth();
    _frameHeight = currFrame->getHeight();


    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        currFrame->getGLTextureInternalFormat(),
        _frameWidth,
        _frameHeight,
        0,
        currFrame->getGLTextureFormat(),
        currFrame->getGLTextureType(),
        currFrame->data()
    );
    
    if (_autoFit) {
        resetZoom();
    }


    _program->bind();

    QMatrix4x4 m;
    _program->setUniformValue("matrix", m);


    //
    // Image and Viewport dimensions
    //
    
    _program->setUniformValue("viewport", _viewportX, _viewportY);
    _program->setUniformValue("image", _frameWidth, _frameHeight);

    QVariantMap bm = buf->getBufferMetadataSafeCopy();
    QString layoutMode = bm.value("composite_mode").toString();

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
    }

    _program->setUniformValue("layout_scale", layoutScaleX, layoutScaleY);
    _program->setUniformValue("layout_pan", layoutPanX, layoutPanY);
    
    //
    // Zoom and Pan
    //

    _program->setUniformValue("zoom", getZoom());

    QPointF panPercent = getPanPercent();

    _program->setUniformValue(
        "pan",
        panPercent.x(), panPercent.y());


    //
    // Image channels and color
    //
    
    GLint chan_order = currFrame->getChannelOrder();
    GLint colspace_linear = currFrame->getColourSpace();
    GLfloat opacity = bm.value("opacity").toFloat();
    if (opacity == 0)
        opacity = 1.0;

    // RLP_LOG_DEBUG("opacity:" << opacity << "chan_order:" << chan_order)

    GLfloat alpha = bm.value("alpha").toFloat();
    if (alpha == 0)
        alpha = 1.0;

    _program->setUniformValue("alpha", alpha);
    _program->setUniformValue("chan_order", chan_order);
    _program->setUniformValue("opacity", opacity);

    GLfloat videoGamma = 0.45;
    _program->setUniformValue("videoGamma", videoGamma);

    _program->setUniformValue("do_grayscale", doGreyscale);
    _program->setUniformValue("greyscale_mix", greyscaleMix);

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
    
    
    _program->setUniformValue("texture", 0); // _textureId); // currTextureId);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

}


void
DISP_GLPaintEngine2D::paintGLInternalRecursive(DS_BufferPtr buf, int rec_depth)
{
    // RLP_LOG_DEBUG("")

    if (buf->getDataBytes() != 0)
    {

        // RLP_LOG_DEBUG("BUF DATA BYTES IS NOT ZERO, PAINTING!!");


        QVariantMap bm = buf->getBufferMetadataSafeCopy();
        QString layoutMode = bm.value("composite_mode").toString();

        float pos = -1;
        if (layoutMode == "Wipe")
        {
            pos = bm.value("display_wipe.xpos").toFloat();
            
            int xp = _srect.x() + (_srect.width() * pos) + 1;
            int wp = _srect.width() - (_srect.width() * pos);

            // RLP_LOG_DEBUG("Scissor:" << xp, _srect.y(), wp, _srect.height());

            glScissor(xp, _srect.y(), wp, _srect.height());

            // TODO FIXME: bug in wipe setup - left media draws past
            // wipe position
            glClearColor(0.00, 0.00, 0.00, 1.0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        }

        paintGLInternalOneBuffer(buf);

    }

    // glEnable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    DS_BufferList blist = buf->getAuxBufferList();
    for (int bi=blist.size(); bi !=0; --bi)
    {
        DS_BufferPtr bb = blist.at(bi - 1);
        paintGLInternalRecursive(bb, rec_depth + 1);
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
}


void
DISP_GLPaintEngine2D::setPanEnd()
{
    _panEndPos = getPanPercent();
    _pan = QPointF(0, 0);
}


void
DISP_GLPaintEngine2D::setZoomPos(int zoomX)
{
    _autoFit = false;
    _zoomPos = (float)zoomX / (float)_viewportX;
}


float
DISP_GLPaintEngine2D::getZoom()
{
    // RLP_LOG_DEBUG("DISP_GLPaintEngine2D::getZoom(): " << _zoom + _zoomPos);
    
    return _zoom + _zoomPos;
}


void
DISP_GLPaintEngine2D::setZoomEnd()
{
    _zoom += _zoomPos;
    _zoomPos = 0;
    
    RLP_LOG_DEBUG("zoom end: " << _zoom);

    emit displayZoomChanged(_zoom);
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
    _viewportY = (float)height; // = glm::vec2((float)width, (float)height);
    
    // NOTE: this seems to be getting called twice on every resizeGL() call in GlslWidget?
    // RLP_LOG_DEBUG(width << " " << height);

    resetZoom();
}


/*
void
DISP_GLPaintEngine2D::setPan(int xpos, int ypos)
{
    RLP_LOG_DEBUG("DISP_GLPaintEngine2D::setPan(): " << xpos << " " << ypos);
    
    _panX = xpos;
    _panY = ypos;
}
*/



void
DISP_GLPaintEngine2D::resetZoom()
{

    float zoomx = ((float)(_viewportX)) / (float)_frameWidth;
    float zoomy = ((float)(_viewportY)) / (float)_frameHeight;
    
    _zoom = std::min(zoomx, zoomy);
    
    _autoFit = true;

    emit displayZoomChanged(_zoom);

    // RLP_LOG_DEBUG("_zoom: " << _zoom);
}
