#ifndef REVLENS_VIDEO_SYNCTARGET_H
#define REVLENS_VIDEO_SYNCTARGET_H

#include "RlpRevlens/VIDEO/Global.h"

class REVLENS_VIDEO_API VIDEO_SyncTarget : public QObject {
    Q_OBJECT
    
public:

    VIDEO_SyncTarget() {}
    ~VIDEO_SyncTarget() {}
    
    virtual void displayNextFrame() {}
};


#endif
