//
//  Copyright (c) 2014 Justin Ottley. All rights reserved.
//

#ifdef _WIN32
#define OPENEXR_DLL
#endif

#include "RlpExtrevlens/RLEXRDS/FrameBuffer.h"

#include <OpenEXR/ImfInputFile.h>
#include <OpenEXR/ImfOutputFile.h>
#include <OpenEXR/ImfTiledOutputFile.h>
#include <OpenEXR/ImfRgbaFile.h>
#include <OpenEXR/ImfPreviewImage.h>
#include <OpenEXR/ImfArray.h>

#include <ImathMath.h>
#include <ImathFun.h>
#include <half.h>

#include <math.h>
#include <iostream>
#include <algorithm>

using namespace OPENEXR_IMF_NAMESPACE;
using namespace IMATH_NAMESPACE;
using namespace std;


RLP_SETUP_LOGGER(ext, RLEXRDS, FrameBuffer)


float
preview_knee(float x, float f)
{
    return log (x * f + 1) / f;
}


unsigned char
preview_gamma(half h, float m)
{
    //
    // Conversion from half to unsigned char pixel data,
    // with gamma correction.  The conversion is the same
    // as in the exrdisplay program's ImageView class,
    // except with defog, kneeLow, and kneeHigh fixed
    // at 0.0, 0.0, and 5.0 respectively.
    //

    float x = max (0.f, h * m);

    if (x > 1)
	x = 1 + preview_knee (x - 1, 0.184874f);

    return (unsigned char) (IMATH_NAMESPACE::clamp (Math<float>::pow (x, 0.4545f) * 84.66f, 
				   0.f,
				   255.f));
}


void
generatePreview (const char inFileName[],
		 float exposure,
		 int previewWidth,
		 int &previewHeight,
		 Array2D <PreviewRgba> &previewPixels)
{
    //
    // Read the input file
    //

    RgbaInputFile in (inFileName);

    Box2i dw = in.dataWindow();
    float a = in.pixelAspectRatio();
    int w = dw.max.x - dw.min.x + 1;
    int h = dw.max.y - dw.min.y + 1;

    Array2D <Rgba> pixels (h, w);
    in.setFrameBuffer (&pixels[0][0] - dw.min.y * w - dw.min.x, 1, w);
    in.readPixels (dw.min.y, dw.max.y);

    //
    // Make a preview image
    //

    previewHeight = max (int (h / (w * a) * previewWidth + .5f), 1);
    previewPixels.resizeErase (previewHeight, previewWidth);

    float fx = (previewWidth  > 0)? (float (w - 1) / (previewWidth  - 1)): 1;
    float fy = (previewHeight > 0)? (float (h - 1) / (previewHeight - 1)): 1;
    float m  = Math<float>::pow (2.f, IMATH_NAMESPACE::clamp (exposure + 2.47393f, -20.f, 20.f));

    for (int y = 0; y < previewHeight; ++y)
    {
	for (int x = 0; x < previewWidth; ++x)
	{
	    PreviewRgba &preview = previewPixels[y][x];
	    const Rgba &pixel = pixels[int (y * fy + .5f)][int (x * fx + .5f)];

	    preview.r = preview_gamma (pixel.r, m);
	    preview.g = preview_gamma (pixel.g, m);
	    preview.b = preview_gamma (pixel.b, m);
	    preview.a = int (IMATH_NAMESPACE::clamp (pixel.a * 255.f, 0.f, 255.f) + .5f);
	}
    }
}


void
RLEXRDS_FrameBuffer::reallocateHalf()
{
    // RLP_LOG_DEBUG("")

    // Imath::Box2i dw = _bufferMetadata.value("dataWindow").value<Imath::Box2i>();

    int dw_max_x = _bufferMetadata.value("dw.max.x").value<int>();
    int dw_max_y = _bufferMetadata.value("dw.max.y").value<int>();
    int dw_min_x = _bufferMetadata.value("dw.min.x").value<int>();
    int dw_min_y = _bufferMetadata.value("dw.min.y").value<int>();

    _width  = dw_max_x - dw_min_x + 1;
    _height = dw_max_y - dw_min_y + 1;
    
    // std::cout << "EXR FRAME BUFFER ALLOCATING " << _width << " " << _height << std::endl;
    
    int ySampling = 1;
    int xSampling = 1;
    
    
    size_t channelCount = 4;
    
    // std::cout << "CHANNEL COUNT: " << channelCount << std::endl;
    
    size_t pixelSize = sizeof(half) * channelCount;
    size_t lineSize = pixelSize * _width;
    
    _dataBytes = _width * _height * pixelSize;
    
    _data = new char[_dataBytes];
    
    char* r_pixel = ((char *)_data);
    char* g_pixel = ((char *)_data) + sizeof(half);
    char* b_pixel = ((char *)_data) + (sizeof(half) * 2);
    char* a_pixel = ((char *)_data) + (sizeof(half) * 3);
    
    _frameBuffer.insert("R",
        Imf::Slice(Imf::HALF,
            r_pixel - (dw_min_y / ySampling) * lineSize -
                (dw_min_x / xSampling) * pixelSize,
            pixelSize,
            lineSize,
            xSampling,
            ySampling));
    
    _frameBuffer.insert("G",
        Imf::Slice(Imf::HALF,
            g_pixel - (dw_min_y / ySampling) * lineSize -
                (dw_min_x / xSampling) * pixelSize,
            pixelSize,
            lineSize,
            xSampling,
            ySampling));
    
    _frameBuffer.insert("B",
        Imf::Slice(Imf::HALF,
            b_pixel - (dw_min_y / ySampling) * lineSize -
                (dw_min_x / xSampling) * pixelSize,
            pixelSize,
            lineSize,
            xSampling,
            ySampling));
    
        
    _frameBuffer.insert("A",
        Imf::Slice(Imf::HALF,
            a_pixel - (dw_min_y / ySampling) * lineSize -
                (dw_min_x / xSampling) * pixelSize,
            pixelSize,
            lineSize,
            xSampling,
            ySampling));

}


void
RLEXRDS_FrameBuffer::reallocateFloat()
{
        
    // Imath::Box2i dw = _bufferMetadata.value("dataWindow").value<Imath::Box2i>();

    int dw_max_x = _bufferMetadata.value("dw.max.x").value<int>();
    int dw_max_y = _bufferMetadata.value("dw.max.y").value<int>();
    int dw_min_x = _bufferMetadata.value("dw.min.x").value<int>();
    int dw_min_y = _bufferMetadata.value("dw.min.y").value<int>();


    _width  = dw_max_x - dw_min_x + 1;
    _height = dw_max_y - dw_min_y + 1;
    
    // std::cout << "EXR FRAME BUFFER ALLOCATING " << _width << " " << _height << std::endl;
    
    int ySampling = 1;
    int xSampling = 1;
    
    
    size_t channelCount = 4;
    
    // size_t channelCount = _bufferMetadata.value("channelCount").value<int>();
    // std::cout << "CHANNEL COUNT: " << channelCount << std::endl;
    
    size_t pixelSize = sizeof(Imf::FLOAT) * channelCount;
    size_t lineSize = pixelSize * _width;
    
    _dataBytes = _width * _height * pixelSize;
    
    _data = new char[_dataBytes];
    
    char* r_pixel = ((char *)_data);
    char* g_pixel = ((char *)_data) + sizeof(Imf::FLOAT);
    char* b_pixel = ((char *)_data) + (sizeof(Imf::FLOAT) * 2);
    char* a_pixel = ((char *)_data) + (sizeof(Imf::FLOAT) * 3);
    
    _frameBuffer.insert("R",
        Imf::Slice(Imf::FLOAT,
            r_pixel - (dw_min_y / ySampling) * lineSize -
                (dw_min_x / xSampling) * pixelSize,
            pixelSize,
            lineSize,
            xSampling,
            ySampling));
    
    _frameBuffer.insert("G",
        Imf::Slice(Imf::FLOAT,
            g_pixel - (dw_min_y / ySampling) * lineSize -
                (dw_min_x / xSampling) * pixelSize,
            pixelSize,
            lineSize,
            xSampling,
            ySampling));
    
    _frameBuffer.insert("B",
        Imf::Slice(Imf::FLOAT,
            b_pixel - (dw_min_y / ySampling) * lineSize -
                (dw_min_x / xSampling) * pixelSize,
            pixelSize,
            lineSize,
            xSampling,
            ySampling));
    
        
    _frameBuffer.insert("A",
        Imf::Slice(Imf::FLOAT,
            a_pixel - (dw_min_y / ySampling) * lineSize -
                (dw_min_x / xSampling) * pixelSize,
            pixelSize,
            lineSize,
            xSampling,
            ySampling));

}


void
RLEXRDS_FrameBuffer::reallocate()
{
    _pixelSize = _bufferMetadata.value("pixelSize").toInt();

    // setChannelOrder(DS_FrameBuffer::CHANNEL_ORDER_RGBA);
    setGLTextureFormat(GL_RGBA);
    setGLTextureInternalFormat(GL_RGBA);
    
    RLP_LOG_DEBUG(_pixelSize)

    if (_pixelSize == 2)
    {
        reallocateHalf();
        setGLTextureType(GL_HALF_FLOAT);

    }
    else if (_pixelSize == 4)
    {
        reallocateFloat();
        setGLTextureType(GL_FLOAT);
    }
}



QImage
RLEXRDS_FrameBuffer::toQImage()
{
    QString srcPath = _bufferMetadata.value("exr.path").toString();

    // RLP_LOG_DEBUG(srcPath)

    Array2D <PreviewRgba> previewPixels;
    int previewHeight;

    float exposure = 0.0;

    generatePreview(
        srcPath.toUtf8(),
        exposure,
        _width,
        previewHeight,
        previewPixels);


    QImage i(_width, _height, QImage::Format_RGBA8888);

    for (int y = 0; y < _height; y++) {
        for (int x = 0; x < _width; x++) {
            PreviewRgba &preview = previewPixels[y][x];
            i.setPixel(x, y,
                qRgba(preview.r, preview.g, preview.b, preview.a)
            );
        }
    }
    
    return i;

}