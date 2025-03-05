
#include "RlpExtrevlens/RLOCIO/PaintEngine.h"


namespace OCIO = OCIO_NAMESPACE;


static const std::array<std::pair<const char*, OCIO::OptimizationFlags>, 5> OptmizationMenu = { {
    { "None",      OCIO::OPTIMIZATION_NONE },
    { "Lossless",  OCIO::OPTIMIZATION_LOSSLESS },
    { "Very good", OCIO::OPTIMIZATION_VERY_GOOD },
    { "Good",      OCIO::OPTIMIZATION_GOOD },
    { "Draft",     OCIO::OPTIMIZATION_DRAFT } } };

bool g_verbose = true;

RLP_SETUP_LOGGER(extrevlens, RLOCIO, PaintEngine)

RLOCIO_PaintEngine::RLOCIO_PaintEngine():
    DISP_GLPaintEngine2D(),
    _rebuild(true),
    _inputColorSpace(OCIO::ROLE_SCENE_LINEAR),
    _display(""),
    _transformName(""),
    _look(""),
    _exposureFstop(0.0f),
    _displayGamma(1.0f),
    _channelHot{1, 1, 1, 1},
    _optimization(OCIO::OPTIMIZATION_DEFAULT)
{
    _name = "OCIO";

    char* glslVersion = (char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
    char* glVer = (char*)glGetString(GL_VERSION);

    RLP_LOG_DEBUG("Initalizing - GLSL version:" << glslVersion << "OpenGL Version:" << glVer)
    
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


    initOCIO();
}


DISP_GLPaintEngine2D*
RLOCIO_PaintEngine::create()
{
    return new RLOCIO_PaintEngine();
}


void
RLOCIO_PaintEngine::initOCIO()
{
    RLP_LOG_DEBUG("")

    OCIO::ConstConfigRcPtr config = OCIO::GetCurrentConfig();

    _display = config->getDefaultDisplay();
    _transformName = config->getDefaultView(_display.c_str());
    _look = config->getDisplayViewLooks(_display.c_str(), _transformName.c_str());

    /*
    _metadata.insert("ocio.image_colorspace", QString(_inputColorSpace.c_str()));
    _metadata.insert("ocio.display", QString(_display.c_str()));
    _metadata.insert("ocio.transform", QString(_transformName.c_str()));
    _metadata.insert("ocio.look", QString(_look.c_str()));
    */
}


OCIO::GpuShaderDescRcPtr
RLOCIO_PaintEngine::buildOCIOShaderDescription()
{
    RLP_LOG_DEBUG("")


    // Step 0: Get the processor using any of the pipelines mentioned above.
    OCIO::ConstConfigRcPtr config = OCIO::GetCurrentConfig();

    OCIO::DisplayViewTransformRcPtr transform = OCIO::DisplayViewTransform::Create();
    transform->setSrc(_inputColorSpace.c_str());
    transform->setDisplay(_display.c_str());
    transform->setView(_transformName.c_str());

    OCIO::LegacyViewingPipelineRcPtr vp = OCIO::LegacyViewingPipeline::Create();
    vp->setDisplayViewTransform(transform);
    vp->setLooksOverrideEnabled(true);
    vp->setLooksOverride(_look.c_str());

    if (g_verbose)
    {
        std::cout << std::endl;
        std::cout << "Color transformation composed of:" << std::endl;
        std::cout << "      Image ColorSpace is:\t" << _inputColorSpace << std::endl;
        std::cout << "      Transform is:\t\t" << _transformName << std::endl;
        std::cout << "      Device is:\t\t" << _display << std::endl;
        std::cout << "      Looks Override is:\t'" << _look << "'" << std::endl;
        std::cout << "  with:" << std::endl;
        std::cout << "    exposure_fstop = " << _exposureFstop << std::endl;
        std::cout << "    display_gamma  = " << _displayGamma << std::endl;
        std::cout << "    channels       = " 
                  << (_channelHot[0] ? "R" : "")
                  << (_channelHot[1] ? "G" : "")
                  << (_channelHot[2] ? "B" : "")
                  << (_channelHot[3] ? "A" : "") << std::endl;


        for (const auto & opt : OptmizationMenu)
        {
            if (opt.second == _optimization)
            {
                std::cout << std::endl << "Optimization: " << opt.first << std::endl;
            }
        }


    }

    // Add optional transforms to create a full-featured, "canonical" display pipeline
    // Fstop exposure control (in SCENE_LINEAR)
    {
        double gain = powf(2.0f, _exposureFstop);
        const double slope4f[] = { gain, gain, gain, gain };
        double m44[16];
        double offset4[4];
        OCIO::MatrixTransform::Scale(m44, offset4, slope4f);
        OCIO::MatrixTransformRcPtr mtx =  OCIO::MatrixTransform::Create();
        mtx->setMatrix(m44);
        mtx->setOffset(offset4);
        vp->setLinearCC(mtx);
    }

    // Channel swizzling
    {
        double lumacoef[3];
        config->getDefaultLumaCoefs(lumacoef);
        double m44[16];
        double offset[4];
        OCIO::MatrixTransform::View(m44, offset, _channelHot, lumacoef);
        OCIO::MatrixTransformRcPtr swizzle = OCIO::MatrixTransform::Create();
        swizzle->setMatrix(m44);
        swizzle->setOffset(offset);
        vp->setChannelView(swizzle);
    }

    // Post-display transform gamma
    {
        double exponent = 1.0/std::max(1e-6, static_cast<double>(_displayGamma));
        const double exponent4f[4] = { exponent, exponent, exponent, exponent };
        OCIO::ExponentTransformRcPtr expTransform =  OCIO::ExponentTransform::Create();
        expTransform->setValue(exponent4f);
        vp->setDisplayCC(expTransform);
    }

    OCIO::ConstProcessorRcPtr processor;
    try
    {
        processor = vp->getProcessor(config, config->getCurrentContext());
    }
    catch (const OCIO::Exception & e)
    {
        std::cerr << e.what() << std::endl;
        return nullptr;
    }
    catch (...)
    {
        return nullptr;
    }

    // Set the shader context.
    OCIO::GpuShaderDescRcPtr shaderDesc = OCIO::GpuShaderDesc::CreateShaderDesc();
    shaderDesc->setLanguage(OCIO::GPU_LANGUAGE_GLSL_1_2);
    shaderDesc->setFunctionName("OCIODisplay");
    shaderDesc->setResourcePrefix("ocio_");

    // Extract the shader information.
    OCIO::ConstGPUProcessorRcPtr gpu = processor->getOptimizedGPUProcessor(_optimization);

    gpu->extractGpuShaderInfo(shaderDesc);

    return shaderDesc;
}


void
RLOCIO_PaintEngine::rebuildOCIOGLProgram()
{
    RLP_LOG_DEBUG("")

    if (!_rebuild)
    {
        RLP_LOG_WARN("rebuild flag not on, skipping")
        return;
    }


    if (_vshader != nullptr)
    {
        delete _vshader;
    }

    if (_fshader != nullptr)
    {
        delete _fshader;
    }

    
    if (_oglBuilder != nullptr)
    {
        // TODO FIXME: unstable. memory leak?
        // delete _oglBuilder;
    }
    

    if (_program != nullptr)
    {
        delete _program;
    }

    //
    // Init main shader program
    //
    
    _program = new QOpenGLShaderProgram(this);
    _vshader = new QOpenGLShader(QOpenGLShader::Vertex, this);
    _vshader->compileSourceCode(vertex_shader_2D);
    _program->addShader(_vshader);

    #define PROGRAM_VERTEX_ATTRIBUTE 0
    #define PROGRAM_TEXCOORD_ATTRIBUTE 1

    _program->bindAttributeLocation("vertex", PROGRAM_VERTEX_ATTRIBUTE);
    _program->bindAttributeLocation("texCoord", PROGRAM_TEXCOORD_ATTRIBUTE);

    //
    // Init OCIO Fragment Shader
    //

    OCIO::GpuShaderDescRcPtr shaderDesc = buildOCIOShaderDescription();

    _oglBuilder = new OCIO::OpenGLBuilder(
        shaderDesc,
        _program->programId()
    );

    _oglBuilder->setVerbose(true);


    //
    // Init OCIO Textures
    //

    // RLP_LOG_DEBUG("Allocating OCIO Textures")
    _oglBuilder->allocateAllTextures(1);

    //
    // Assemble fragment shader
    //
    QString fshaderText("#version 120\n\n");
    fshaderText += shaderDesc->getShaderText();
    fshaderText += OCIO_fragment_shader_2D;


    _fshader = new QOpenGLShader(QOpenGLShader::Fragment, this);
    _fshader->compileSourceCode(fshaderText);

    // RLP_LOG_DEBUG("shader log:");
    // RLP_LOG_DEBUG(_fshader->log());

    _program->addShader(_fshader);

    _program->link();

    _program->bind();

    _rebuild = false;
}


void
RLOCIO_PaintEngine::initializeGLInternal(DS_BufferPtr buf)
{
    // RLP_LOG_DEBUG("")

    DS_FrameBuffer* currFrame = dynamic_cast<DS_FrameBuffer* >(buf.get());
    
    // RLP_LOG_DEBUG("buffer: " << currFrame->getDataBytes())
    _enabled = currFrame->getColourSpace() == DS_FrameBuffer::COLSPACE_LINEAR;

    if (currFrame->getDataBytes() == 0)
    {
        RLP_LOG_WARN("No framebuffer, skipping")
        return;
    }

    if (_rebuild)
    {
        /*
        GLint currTextureType = getCurrentGlTextureType(currFrame);
        
        // RLP_LOG_DEBUG("curr texture type: " << currTextureType)

        if (currTextureType == GL_HALF_FLOAT)
        {
            RLP_LOG_DEBUG("HALF FLOAT DETECTED!!");
        }
        else if (currTextureType == GL_UNSIGNED_BYTE)
        {
            RLP_LOG_DEBUG("UNSIGNED BYTE DETECTED!!")
        }
        */

        glGenTextures(1, &_textureId);

        rebuildOCIOGLProgram();

    }


    glBindTexture(GL_TEXTURE_2D, _textureId);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


    //
    // OCIO - Bind textures and values
    //
    _oglBuilder->useAllTextures();
    _oglBuilder->useAllUniforms();

    // RLP_LOG_DEBUG("texture bind : " << _textureId << " " << _frameWidth << " " << _frameHeight);



    _program->setUniformValue("texture", 0); // _textureId); // 0); // currTextureId);
    
    GLint val = 0;
    _program->setUniformValue("chan_order", val);
    
    GLfloat videoGamma = 0.45;
    _program->setUniformValue("videoGamma", videoGamma);

}


void
RLOCIO_PaintEngine::setInputColorSpace(QString cspace)
{
    RLP_LOG_DEBUG(cspace)

    _inputColorSpace = cspace.toUtf8().constData();
    _rebuild = true;
}


void
RLOCIO_PaintEngine::setInputColorSpaceByIndex(int idx)
{
    OCIO::ConstConfigRcPtr config = OCIO::GetCurrentConfig();
    const char* name = config->getColorSpaceNameByIndex(idx);
    
    std::string newName(name);

    RLP_LOG_DEBUG(name)
    _inputColorSpace = newName;
    _rebuild = true;
    
}

void
RLOCIO_PaintEngine::setTransform(QString transform)
{
    RLP_LOG_DEBUG(transform)

    _transformName = transform.toUtf8().constData();
    _rebuild = true;
}


void
RLOCIO_PaintEngine::setDisplay(QString display)
{
    RLP_LOG_DEBUG(display)

    _display = display.toUtf8().constData();
    _rebuild = true;
}


void
RLOCIO_PaintEngine::setExposure(float val)
{
    _exposureFstop = val;
    _rebuild = true;
}


void
RLOCIO_PaintEngine::setGamma(float val)
{
    _displayGamma = val;
    _rebuild = true;
}


void
RLOCIO_PaintEngine::setChannelR()
{
    _channelHot[0] = 1;
    _channelHot[1] = 0;
    _channelHot[2] = 0;
    _channelHot[3] = 0;
    
    _rebuild = true;
}

void
RLOCIO_PaintEngine::setChannelG()
{
    _channelHot[0] = 0;
    _channelHot[1] = 1;
    _channelHot[2] = 0;
    _channelHot[3] = 0;
    
    _rebuild = true;
}


void
RLOCIO_PaintEngine::setChannelB()
{
    _channelHot[0] = 0;
    _channelHot[1] = 0;
    _channelHot[2] = 1;
    _channelHot[3] = 0;
    
    _rebuild = true;
}


void
RLOCIO_PaintEngine::setChannelRGB()
{
    _channelHot[0] = 1;
    _channelHot[1] = 1;
    _channelHot[2] = 1;
    _channelHot[3] = 1;
    
    _rebuild = true;
}

