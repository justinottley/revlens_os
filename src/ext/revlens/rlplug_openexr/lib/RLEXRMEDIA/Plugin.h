//
//  Copyright (c) 2014 Justin Ottley. All rights reserved.
//

#ifndef EXTREVLENS_RLEXRMEDIA_PLUGIN_H
#define EXTREVLENS_RLEXRMEDIA_PLUGIN_H


#include "RlpExtrevlens/RLEXRMEDIA/Global.h"

#include "RlpRevlens/DS/Node.h"
#include "RlpRevlens/MEDIA/Plugin.h"

class RLEXRMEDIA_VideoReader;

class EXTREVLENS_RLEXRMEDIA_API RLEXRMEDIA_Plugin : public MEDIA_Plugin {
    Q_OBJECT
    
public:
    RLP_DEFINE_LOGGER
    
    RLEXRMEDIA_Plugin();

    DS_NodePtr createVideoReader(QVariantMap* properties);

    DS_FrameBufferPtr getOrCreateFrameBuffer(QObject* owner, QVariantMap frameInfo , bool forceNew=false);


public slots:
    QVariantList identifyMedia(QVariantMap mediaInput, DS_Node::NodeDataType dataType=DS_Node::DATA_UNKNOWN);
    
    int getReaderThreads() { return _readerThreads; }
    int getImfThreads() { return _imfThreads; }

    QString getSettingsUI() { return QString("rlplug_openexr.panel.settings"); }

private:
    int _readerThreads;
    int _imfThreads;
    
};


Q_DECLARE_METATYPE(RLEXRMEDIA_Plugin *)


class EXTREVLENS_RLEXRMEDIA_API RLEXRMEDIA_ReadTask : public QRunnable {
    
public:
    
    RLEXRMEDIA_ReadTask(RLEXRMEDIA_VideoReader* reader,
                          DS_TimePtr timeInfo,
                          DS_BufferPtr destFrame);
    
    void run();
    
private:
    
    RLEXRMEDIA_VideoReader* _reader;
    DS_TimePtr _timeInfo;
    DS_BufferPtr _destFrame;

};

#endif

