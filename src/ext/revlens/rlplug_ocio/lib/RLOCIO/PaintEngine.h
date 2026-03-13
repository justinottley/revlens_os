
#ifndef EXTREVLENS_RLOCIO_PAINTENGINE_H
#define EXTREVLENS_RLOCIO_PAINTENGINE_H

#include "RlpExtrevlens/RLOCIO/Global.h"
#ifdef _WIN32
#include "RlpExtrevlens/RLOCIO/glsl_22.h"
#else
#include "RlpExtrevlens/RLOCIO/glsl_23.h"
#endif

#include "RlpRevlens/DISP/GLPaintEngine2D.h"
#include "RlpRevlens/DISP/GlslShaders2D.h"

#include <OpenColorIO/OpenColorIO.h>


static const char* OCIO_fragment_shader_2D = R"(

    uniform sampler2D texture;
    varying mediump vec4 texc;
    uniform int chan_order;
    uniform int colspace_linear;

    uniform int do_grayscale;
    uniform float greyscale_mix;

    uniform float alpha;
    uniform float opacity;
    uniform float videoGamma;

    vec3 greyscale(vec3 color, float alphaval) {
        float g = dot(color, vec3(0.299, 0.587, 0.114));
        return mix(color, vec3(g), alphaval);
    }

    vec4 greyscale(vec4 color, float gmix) {
        vec3 cresult = greyscale(color.rgb, gmix);
        vec4 result = vec4(cresult.rgb.r, cresult.rgb.g, cresult.rgb.b, color.a);
        return result;
    }

    vec3 XYZ_to_LinearRGB(vec3 xyz) {
        float r = 3.2404542 * xyz.x - 1.5371385 * xyz.y - 0.4985314 * xyz.z;
        float g = -0.9692660 * xyz.x + 1.8760108 * xyz.y + 0.0415560 * xyz.z;
        float b = 0.0556434 * xyz.x - 0.2040259 * xyz.y + 1.0572252 * xyz.z;
        return vec3(r, g, b);
    }

    void main()
    {
        int corder = 1;
        highp vec4 color;

        if (chan_order == 0)
        {
            color = vec4(texture2D(texture, texc.st).bgr, alpha); // , texture2D(texture, texc.st).a);
        }
        else if (chan_order == 1)
        {
            color = vec4(texture2D(texture, texc.st).rgba);
        }
        else if (chan_order == 2)
        {
            color = vec4(texture2D(texture, texc.st).rgb, alpha);
        }
        
        else if (chan_order == 3)
        {
            color = texture2D(texture, texc.st);
            
        }

        color = vec4(opacity, opacity, opacity, opacity) * color;

        if (do_grayscale == 1)
        {
            color = greyscale(color, greyscale_mix);
            // red tint
            // color = vec4(color.rgb.r, 0, 0, color.a);
        }


        if (colspace_linear == 1)
        {
            color = OCIODisplay(color);

        } else if (colspace_linear == 2)
        {
            vec3 cresult = XYZ_to_LinearRGB(color.rgb);
            color = vec4(cresult.r, cresult.g, cresult.b, color.a);

            // vec3 rresult = LinearRGB_to_sRGB(cresult);
            // color = vec4(rresult.r, rresult.g, rresult.b, color.a);
        }
        
        gl_FragColor = color;
    })";


class EXTREVLENS_RLOCIO_API RLOCIO_PaintEngine : public DISP_GLPaintEngine2D {
    Q_OBJECT


public:
    RLP_DEFINE_LOGGER

    RLOCIO_PaintEngine();

    void initOCIO();

    OCIO_NAMESPACE::GpuShaderDescRcPtr buildOCIOShaderDescription();

    void rebuildOCIOGLProgram();

    static DISP_GLPaintEngine2D* create();

    void initializeGLInternal(DS_BufferPtr buf);

public slots:

    std::string inputColorSpace() { return _inputColorSpace; }
    void setInputColorSpace(QString cspace);
    void setInputColorSpaceByIndex(int idx);

    std::string transform() { return _transformName; }
    void setTransform(QString transform);

    std::string display() { return _display; }
    void setDisplay(QString display);

    float exposure() { return _exposureFstop; }
    void setExposure(float exposure);

    float gamma() { return _displayGamma; }
    void setGamma(float gamma);

    void setChannelR();
    void setChannelG();
    void setChannelB();
    void setChannelRGB();

    bool isEnabled() { return _enabled; }

protected:

    bool _rebuild;
    bool _enabled;

    std::string _inputColorSpace;

    std::string _display;
    std::string _transformName;
    std::string _look;

    float _exposureFstop;
    float _displayGamma;
    int _channelHot[4];

    OCIO_NAMESPACE::OptimizationFlags _optimization;
    OCIO_NAMESPACE::OpenGLBuilder* _oglBuilder;

};

Q_DECLARE_METATYPE(RLOCIO_PaintEngine*)


#endif