//
//  Copyright (c) 2014 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_MEDIA_PLUGIN_H
#define REVLENS_MEDIA_PLUGIN_H


#include "RlpRevlens/MEDIA/Global.h"

#include "RlpCore/LOG/Logging.h"

#include "RlpRevlens/DS/Plugin.h"
#include "RlpRevlens/DS/FrameBuffer.h"
#include "RlpRevlens/DS/Node.h"



class MEDIA_Plugin;
typedef std::shared_ptr<MEDIA_Plugin> MEDIA_PluginPtr;


class REVLENS_MEDIA_API MEDIA_Plugin : public DS_Plugin {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    enum MediaType {
        VIDEO_FRAMES, // one file per frame
        VIDEO_CONTAINER, // mov mp4 avi etc
        AUDIO_FILE, // standalone audio file
        AUDIO_CONTAINER, // audio embedded in a media container
        UNKNOWN
    };


    MEDIA_Plugin(QString name, int readThreadPoolSize = 1);

    virtual QThreadPool* getReadThreadPool(DS_Node::NodeDataType mediaDataType);

    virtual DS_FrameBufferPtr getOrCreateFrameBuffer(QObject* owner, QVariantMap frameInfo, bool forceNew=false);

    virtual DS_NodePtr createVideoReader(QVariantMap* properties) { return nullptr; }
    virtual DS_NodePtr createAudioReader(QVariantMap* properties) { return nullptr; }

public slots:

    QString name() { return _name; } // redefined for binding

    virtual QVariantList identifyMedia(QVariantMap mediaInput, DS_Node::NodeDataType=DS_Node::DATA_UNKNOWN);

    virtual int getBytesRequiredForFrame(DS_NodePtr node, qlonglong frameNum);

    virtual QString getSettingsUI() { return QString(""); }



protected:

    DS_FrameBufferPtr getPreallocatedFrameBuffer(QVariantMap frameInfo);

    QMutex* _lock;
    QMap<DS_Node::NodeDataType, QThreadPool*> _readThreadPoolMap;
    bool _composable;
};

Q_DECLARE_METATYPE(MEDIA_Plugin *)


class MEDIA_ReadTask : public QRunnable {

public:
    MEDIA_ReadTask(DS_NodePtr node, int frameNum, DS_BufferPtr destFrame);

    void run();

private:

    DS_NodePtr _node;
    int frameNum;
    DS_FrameBufferPtr destFrame;
};


#endif

