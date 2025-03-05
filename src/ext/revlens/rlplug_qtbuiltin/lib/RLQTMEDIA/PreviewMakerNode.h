//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef EXTREVLENS_RLQTMEDIA_PREVIEW_MAKER_NODE_H
#define EXTREVLENS_RLQTMEDIA_PREVIEW_MAKER_NODE_H

#include "RlpExtrevlens/RLQTMEDIA/Global.h"

#include "RlpRevlens/DS/Node.h"



class EXTREVLENS_RLQTMEDIA_API RLQTMEDIA_PreviewMakerNode : public DS_Node {
    Q_OBJECT

signals:
    void imageReady(long frame, QImage image);

public:
    RLP_DEFINE_LOGGER

    RLQTMEDIA_PreviewMakerNode();
    RLQTMEDIA_PreviewMakerNode(QVariantMap* properties);
    
    void readVideo(DS_TimePtr timeInfo, DS_FrameBufferPtr destFrame, bool postUpdate = true);
    void scheduleReadTask(DS_TimePtr timeInfo, DS_FrameBufferPtr destFrame, bool optional = false) {}


public slots:

    static RLQTMEDIA_PreviewMakerNode* new_RLQTMEDIA_PreviewMakerNode(DS_NodePtr input);

    QImage makePreview(long frameNum);
    // void scheduleReadTask(long frameNum);


};


class EXTREVLENS_RLQTMEDIA_API RLQTMEDIA_PreviewMakerReadTask : public QRunnable {

public:
    
    RLQTMEDIA_PreviewMakerReadTask(RLQTMEDIA_PreviewMakerNode* reader,
                                   DS_TimePtr timeInfo);

    void run();
    
private:
    
    RLQTMEDIA_PreviewMakerNode* _reader;
    DS_TimePtr _timeInfo;
    DS_FrameBufferPtr _destFrame;
};


/*
class EXTREVLENS_RLQTMEDIA_API RLQTMEDIA_PreviewMakerNodeDecorator : public QObject {
    Q_OBJECT
    
public slots:
    
    DS_NodePtr* new_RLQTMEDIA_PreviewMakerNodePtr() {
        return new DS_NodePtr(new RLQTMEDIA_PreviewMakerNode());
    }
    
    RLQTMEDIA_PreviewMakerNode* new_RLQTMEDIA_PreviewMakerNode(DS_NodePtr input) {
        RLQTMEDIA_PreviewMakerNode* node = new RLQTMEDIA_PreviewMakerNode(input->getPropertiesPtr());
        node->addInput(input);
        
        return node;
    }
};
*/


#endif

