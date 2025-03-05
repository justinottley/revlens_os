//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef EXTREVLENS_RLANN_DS_ANNOTATION_RELEASE_TASK_H
#define EXTREVLENS_RLANN_DS_ANNOTATION_RELEASE_TASK_H

#include "RlpExtrevlens/RLANN_DS/Global.h"
#include "RlpExtrevlens/RLANN_DS/Annotation.h"
#include "RlpExtrevlens/RLANN_DS/DrawControllerBase.h"

class EXTREVLENS_RLANN_DS_API RLANN_DS_AnnotationReleaseTask : public QRunnable {

public:
    RLP_DEFINE_LOGGER

public:

    RLANN_DS_AnnotationReleaseTask(
        RLANN_DS_DrawControllerBase* drawCntrl,
        int trackIdx,
        RLANN_DS_AnnotationPtr ann
    );

    void run();

private:
    RLANN_DS_DrawControllerBase* _drawController;
    int _trackIdx;
    RLANN_DS_AnnotationPtr _ann;
};

class EXTREVLENS_RLANN_DS_API RLANN_DS_AnnotationReloadTask : public QRunnable {

public:
    RLP_DEFINE_LOGGER

public:
    RLANN_DS_AnnotationReloadTask(RLANN_DS_AnnotationPtr ann);

    void run();

private:
    RLANN_DS_AnnotationPtr _ann;

};


#endif