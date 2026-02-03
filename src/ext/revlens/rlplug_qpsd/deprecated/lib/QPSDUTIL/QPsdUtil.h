//
//  Copyright (c) 2014 Justin Ottley. All rights reserved.
//

#ifndef EXTREVLENS_QPSDUTIL_PYMODULE_H
#define EXTREVLENS_QPSDUTIL_PYMODULE_H

class MEDIA_QPsdUtilPyModule {
    
public:
    MEDIA_QPsdUtilPyModule();
    
    
    static int convertPsd(const char* filePath, const char* destPath, int destWidth);
    
};



#endif

