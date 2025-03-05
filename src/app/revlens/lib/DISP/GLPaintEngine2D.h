//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_DISP_PAINTENGINE2D_H
#define REVLENS_DISP_PAINTENGINE2D_H


// #ifndef GL_HALF_FLOAT
// #define GL_HALF_FLOAT QOpenGLTexture::Float16
// #endif

#include "RlpRevlens/DISP/Global.h"

#include "RlpRevlens/DS/Buffer.h"
#include "RlpRevlens/DS/FrameBuffer.h"

#include "RlpRevlens/DISP/GLPaintEngine.h"

class DISP_GLPaintEngine2D;
typedef DISP_GLPaintEngine2D* (*PaintEngineCreateFunc)();
typedef QMap<QString, PaintEngineCreateFunc> PaintEngineCreateMap;

typedef QMap<QString, DISP_GLPaintEngine2D*>::iterator PaintEngineMapIterator;

class REVLENS_DISP_API DISP_GLPaintEngine2D : public DISP_GLPaintEngine {
    Q_OBJECT
    
public:
    RLP_DEFINE_LOGGER
    
public:
    
    DISP_GLPaintEngine2D();

    virtual void initializeGLInternal(DS_BufferPtr buf);
    virtual void paintGLInternalOneBuffer(DS_BufferPtr buf);
    virtual void paintGLInternalRecursive(DS_BufferPtr buf, int rec_depth);
    virtual void paintGLInternal(DS_BufferPtr buf);

    void resizeGLInternal(int width, int height);
    void setSize(int width, int height);

    int getCurrentGlTextureId();

    static DISP_GLPaintEngine2D* create();

public slots:

    QVariantMap metadata() { return _metadata; }

    QPointF mapToImage(QPointF& inImage);
    
    
    void setPanPos(QPointF pan) { _pan = pan; }
    void setPanEnd();
    QPointF getPanPercent();
    
    void resetPan();
    
    void setZoomPos(int zoomX);
    void setZoomEnd();
    float getZoom();

    void setPanZoomEnd() { setPanEnd(); setZoomEnd(); }

    void resetZoom();

    void setDefaultSaturation(float val) { _saturation = val; }

signals:
    void displayZoomChanged(float zoom);

protected:

    // specific to openGL texture paint

    // GLenum _textureFormat;
    // GLuint _textureInternalFormat;
    // GLenum _textureType;

    GLuint _textureId;

    QVector<QVector3D> _vertices;
    QVector<QVector2D> _texCoords;

    QOpenGLShaderProgram* _program;
    QOpenGLShader* _vshader; // vertex shader
    QOpenGLShader* _fshader; // fragment shader


protected:

    float _viewportX;
    float _viewportY;
    
    int _frameWidth;
    int _frameHeight;

    float _zoom;
    float _zoomPos;

    // automatically adjust the zoom per clip to fit image to window
    // turn off if user performs a zoom
    bool _autoFit;

    QPointF _pan;
    QPointF _panEndPos;

    float _saturation; // maybe move this

    QVariantMap _metadata;
};

Q_DECLARE_METATYPE(DISP_GLPaintEngine2D*)

#endif