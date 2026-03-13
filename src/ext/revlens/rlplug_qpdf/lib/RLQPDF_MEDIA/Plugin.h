//
//  Copyright (c) 2014 Justin Ottley. All rights reserved.
//

#ifndef EXTREVLENS_QPDF_PLUGIN_H
#define EXTREVLENS_QPDF_PLUGIN_H

#include "RlpExtrevlens/RLQPDF_MEDIA/Global.h"

#include "RlpRevlens/DS/Node.h"
#include "RlpRevlens/MEDIA/Plugin.h"

#include "RlpGui/BASE/ItemBase.h"

class RLQPDF_MEDIA_VideoReader;

class EXTREVLENS_RLQPDF_MEDIA_API RLQPDF_MEDIA_Plugin : public MEDIA_Plugin {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    RLQPDF_MEDIA_Plugin();

    DS_NodePtr createVideoReader(QVariantMap* properties);
    DS_FrameBufferPtr getOrCreateFrameBuffer(QObject* owner, QVariantMap frameInfo, bool forceNew);


public slots:
    QVariantList identifyMedia(QVariantMap mediaInput, DS_Node::NodeDataType dataType=DS_Node::DATA_UNKNOWN);

    bool writePdf(GUI_ItemBase* widget, QString path, int width, int height);
};


Q_DECLARE_METATYPE(RLQPDF_MEDIA_Plugin *)

class EXTREVLENS_RLQPDF_MEDIA_API RLQPDF_MEDIA_ReadTask : public QRunnable {
    
public:
    
    RLQPDF_MEDIA_ReadTask(
        RLQPDF_MEDIA_VideoReader* reader,
        DS_TimePtr timeInfo,
        DS_FrameBufferPtr destFrame
    );

    void run();
    
private:
    
    RLQPDF_MEDIA_VideoReader* _reader;
    DS_TimePtr _timeInfo;
    DS_FrameBufferPtr _destFrame;
};

#endif

