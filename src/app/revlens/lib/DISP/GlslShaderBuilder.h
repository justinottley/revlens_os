
#ifndef REVLENS_DISP_GLSL_SHADER_BUILDER_H
#define REVLENS_DISP_GLSL_SHADER_BUILDER_H

#include "RlpRevlens/DISP/Global.h"

class REVLENS_DISP_API DISP_GlslShaderBuilder : public QObject
{
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    DISP_GlslShaderBuilder(QString glslVersion="120");

    void registerTextureName(QString name);
    void registerGLFunction(QString funcName, QString funcBody);
    void setGLPostRunFunction(QString func) { _postRunFunction = func; }
    int getGLPostRunFunctionIdx(QString func) { return _glFunctionIdxRevMap.value(func); }

    QString getShaderCode();

private:

    QStringList genTextureNameLines();
    QString genFuncMainAdditionalTextures();
    QString genGLFunctionSwitch();

    // late gen of full func based on number of registered textures
    QString genGLFunction(QString funcName); 

protected:

    QString _glslVersion;
    QList<QString> _textureNames;
    QMap<QString, QString> _glFunctionMap;
    QMap<int, QString> _glFunctionIdxMap;
    QMap<QString, int> _glFunctionIdxRevMap;

    QString _postRunFunction;

};


#endif
