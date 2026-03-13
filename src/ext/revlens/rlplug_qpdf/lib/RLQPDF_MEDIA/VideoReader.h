//
//  Copyright (c) 2014 Justin Ottley. All rights reserved.
//

#ifndef EXTREVLENS_QPDF_MEDIA_VIDEOREADER_H
#define EXTREVLENS_QPDF_MEDIA_VIDEOREADER_H

#include "RlpExtrevlens/RLQPDF_MEDIA/Global.h"

#include "RlpRevlens/DS/Node.h"


class EXTREVLENS_RLQPDF_MEDIA_API RLQPDF_MEDIA_VideoReader : public DS_Node {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    RLQPDF_MEDIA_VideoReader(QVariantMap* properties);

    void readVideo(DS_TimePtr timeInfo, DS_BufferPtr destFrame, bool postUpdate = true);
    void scheduleReadTask(DS_TimePtr timeInfo, DS_FrameBufferPtr destFrame, DS_Node::NodeDataType, bool optional = false);

public slots:

    QImage renderPage(int pageNum);


private:
    QPdfDocument* _doc;

};

#endif

