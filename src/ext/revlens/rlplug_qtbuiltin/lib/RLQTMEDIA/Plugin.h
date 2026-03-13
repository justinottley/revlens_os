//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef EXTREVLENS_RLQTMEDIA_PLUGIN_H
#define EXTREVLENS_RLQTMEDIA_PLUGIN_H

#include <memory>

#include "RlpExtrevlens/RLQTMEDIA/Global.h"

#include "RlpCore/LOG/Logging.h"

#include "RlpRevlens/DS/Node.h"
#include "RlpRevlens/MEDIA/Plugin.h"


class RLQTMEDIA_VideoReader;

class EXTREVLENS_RLQTMEDIA_API RLQTMEDIA_Plugin : public MEDIA_Plugin {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    RLQTMEDIA_Plugin();

    DS_NodePtr createVideoReader(QVariantMap* properties);

public slots:

    QVariantList identifyMedia(QVariantMap mediaInput, DS_Node::NodeDataType dataType=DS_Node::DATA_UNKNOWN);

};



Q_DECLARE_METATYPE(RLQTMEDIA_Plugin *)

class EXTREVLENS_RLQTMEDIA_API RLQTMEDIA_ReadTask : public QRunnable {

public:

    RLQTMEDIA_ReadTask(
        RLQTMEDIA_VideoReader* reader,
        DS_TimePtr timeInfo,
        DS_BufferPtr destFrame
    );

    void run();

private:

    RLQTMEDIA_VideoReader* _reader;
    DS_TimePtr _timeInfo;
    DS_BufferPtr _destFrame;
};

#endif

