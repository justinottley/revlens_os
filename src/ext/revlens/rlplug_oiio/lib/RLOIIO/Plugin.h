
#ifndef EXTREVLENS_RLOIIO_PLUGIN_H
#define EXTREVLENS_RLOIIO_PLUGIN_H

#include "RlpExtrevlens/RLOIIO/Global.h"
#include "RlpRevlens/MEDIA/Plugin.h"

class RLOIIO_VideoReader;

class EXTREVLENS_RLOIIO_API RLOIIO_Plugin : public MEDIA_Plugin {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    RLOIIO_Plugin();

    DS_FrameBufferPtr getOrCreateFrameBuffer(QObject* owner, QVariantMap frameInfo , bool forceNew=false);

    DS_NodePtr createVideoReader(QVariantMap* properties);

public slots:
    QVariantList identifyMedia(QVariantMap mediaInput, DS_Node::NodeDataType dataType=DS_Node::DATA_UNKNOWN);
    QString getSettingsUI() { return QString("rlplug_oiio.settings"); }


private:
    int _readerThreads;

};


Q_DECLARE_METATYPE(RLOIIO_Plugin *)


class EXTREVLENS_RLOIIO_API RLOIIO_ReadTask : public QRunnable {
    
public:
    
    RLOIIO_ReadTask(RLOIIO_VideoReader* reader,
                    DS_TimePtr timeInfo,
                    DS_BufferPtr destFrame);
    
    void run();
    
private:
    
    RLOIIO_VideoReader* _reader;
    DS_TimePtr _timeInfo;
    DS_BufferPtr _destFrame;

};

#endif
