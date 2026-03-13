//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef EXTREVLENS_RLQTMEDIA_DEFERRED_LOAD_PLUGIN_H
#define EXTREVLENS_RLQTMEDIA_DEFERRED_LOAD_PLUGIN_H


#include "RlpExtrevlens/RLQTMEDIA/Global.h"

#include "RlpCore/LOG/Logging.h"

#include "RlpRevlens/MEDIA/Plugin.h"

#include "RlpRevlens/DS/Node.h"
#include "RlpRevlens/DS/QImageFrameBuffer.h"


class RLQTMEDIA_DeferredLoadReader;

class EXTREVLENS_RLQTMEDIA_API RLQTMEDIA_DeferredLoadPlugin : public MEDIA_Plugin {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    RLQTMEDIA_DeferredLoadPlugin();

    DS_NodePtr createVideoReader(QVariantMap* properties);
    DS_NodePtr createAudioReader(QVariantMap* properties);

    DS_FrameBufferPtr getOrCreateFrameBuffer(QObject* owner, QVariantMap frameInfo , bool forceNew=false);

    void setupReadThreadPool(DS_Node::NodeDataType, MEDIA_Plugin* srcPlugin);
    void setFrameInfo(QVariantMap frameInfo) { _frameInfo = frameInfo; }

public slots:

    QVariantList identifyMedia(QVariantMap mediaInput, DS_Node::NodeDataType dataType=DS_Node::DATA_UNKNOWN);

private:
    DS_FrameBufferPtr _loadingFrame;
    QVariantMap _frameInfo;

    QVariantMap _sourceInfo;


};


#endif

