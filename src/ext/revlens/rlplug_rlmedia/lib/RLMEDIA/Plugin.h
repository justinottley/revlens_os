//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef EXTREVLENS_RLMEDIA_PLUGIN_H
#define EXTREVLENS_RLMEDIA_PLUGIN_H

#include <memory>

#include "RlpExtrevlens/RLMEDIA/Global.h"

#include "RlpCore/LOG/Logging.h"

#include "RlpEdb/MEDIA/Controller.h"

#include "RlpRevlens/DS/Node.h"
#include "RlpRevlens/MEDIA/Plugin.h"


class RLMEDIA_VideoReader;

class EXTREVLENS_RLMEDIA_API RLMEDIA_Plugin : public MEDIA_Plugin {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    RLMEDIA_Plugin();

    DS_NodePtr createVideoReader(QVariantMap* properties);
    DS_FrameBufferPtr getOrCreateFrameBuffer(QObject* owner, QVariantMap frameInfo , bool forceNew=false);

public slots:
    
    QVariantList identifyMedia(QVariantMap mediaInput, DS_Node::NodeDataType dataType=DS_Node::DATA_UNKNOWN);

};

Q_DECLARE_METATYPE(RLMEDIA_Plugin *)

class EXTREVLENS_RLMEDIA_API RLMEDIA_ReadTask : public QRunnable {

public:

    RLMEDIA_ReadTask(RLMEDIA_VideoReader* reader,
                     DS_TimePtr timeInfo,
                     DS_BufferPtr destFrame);

    void run();

private:
    
    RLMEDIA_VideoReader* _reader;
    DS_TimePtr _timeInfo;
    DS_BufferPtr _destFrame;
};

#endif

