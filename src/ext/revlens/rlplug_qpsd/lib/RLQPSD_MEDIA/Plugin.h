//
//  Copyright (c) 2014 Justin Ottley. All rights reserved.
//

#ifndef EXTREVLENS_QPSD_PLUGIN_H
#define EXTREVLENS_QPSD_PLUGIN_H

#include "RlpExtrevlens/RLQPSD_MEDIA/Global.h"

#include "RlpRevlens/DS/Node.h"
#include "RlpRevlens/MEDIA/Plugin.h"



class RLQPSD_MEDIA_VideoReader;

class EXTREVLENS_RLQPSD_MEDIA_API RLQPSD_MEDIA_Plugin : public MEDIA_Plugin {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    RLQPSD_MEDIA_Plugin();

    DS_NodePtr createVideoReader(QVariantMap* properties);


public slots:
    QVariantList identifyMedia(QVariantMap mediaInput, DS_Node::NodeDataType dataType=DS_Node::DATA_UNKNOWN);

};


Q_DECLARE_METATYPE(RLQPSD_MEDIA_Plugin *)

class EXTREVLENS_RLQPSD_MEDIA_API RLQPSD_MEDIA_ReadTask : public QRunnable {
    
public:
    
    RLQPSD_MEDIA_ReadTask(RLQPSD_MEDIA_VideoReader* reader,
                         DS_TimePtr timeInfo,
                         DS_FrameBufferPtr destFrame);
    
    void run();
    
private:
    
    RLQPSD_MEDIA_VideoReader* _reader;
    DS_TimePtr _timeInfo;
    DS_FrameBufferPtr _destFrame;
};

#endif

