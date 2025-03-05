//
//  Copyright (c) 2014 Justin Ottley. All rights reserved.
//

#include <iostream>

#include <QtGui/QImage>
#include <QtCore/QFile>

#include "RlpRevlens/QPSD/qpsdhandler.h"
#include "RlpRevlens/QPSDUTIL/QPsdUtil.h"

MEDIA_QPsdUtilPyModule::MEDIA_QPsdUtilPyModule()
{
}


int
MEDIA_QPsdUtilPyModule::convertPsd(const char* filePath, const char* destPath, int destWidth)
{
    std::cout << "Converting PSD \"" << filePath << "\"" << " -> \"" << destPath << "\"" << std::endl;
    
    
    QPsdHandler psdHandler;
    QFile f(filePath);
    
    
    f.open(QIODevice::ReadOnly);
    
    if (!psdHandler.canRead(&f)) {
        
        std::cout << "QPSD: CANNOT READ PSD" << std::endl;
        return 0;
        
    }
    
    
    QImage image;
    
    psdHandler.setDevice(&f);
    psdHandler.read(&image);
    
    
    int width = image.width();
    int height = image.height();
    int channelCount = image.bitPlaneCount() / 8;
    int pixelSize = image.depth() / 8;
    
    // std::cout < "resizing to width " << destWidth << std::endl;
    
    QImage scaledImage = image.scaledToWidth(destWidth, Qt::SmoothTransformation);
    
    scaledImage.save(destPath);
    
    std::cout << "Conversion done" << std::endl;
    
    return 1;
}


