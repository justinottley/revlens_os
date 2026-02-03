
#include "RlpExtrevlens/RLANN_DS/AnnotationTask.h"

#include "RlpExtrevlens/RLANN_DS/Annotation.h"
#include "RlpExtrevlens/RLANN_DS/AnnotationEvent.h"


RLP_SETUP_EXT_LOGGER(RLANN_DS, AnnotationReleaseTask)
RLP_SETUP_EXT_LOGGER(RLANN_DS, AnnotationReloadTask)

RLANN_DS_AnnotationReleaseTask::RLANN_DS_AnnotationReleaseTask(
        RLANN_DS_DrawControllerBase* drawCntrl,
        int trackIdx,
        RLANN_DS_AnnotationPtr ann):
    _drawController(drawCntrl),
    _trackIdx(trackIdx),
    _ann(ann)
{
}


void
RLANN_DS_AnnotationReleaseTask::run()
{
    // RLP_LOG_DEBUG("");

    if (_ann->isDirty()) {
        _ann->offload();
    }
    

    QApplication::postEvent(
        _drawController,
        new RLANN_DS_AnnotationOffloadEvent(
            _trackIdx,
            _ann->uuid().toString()
        )
    );
}


RLANN_DS_AnnotationReloadTask::RLANN_DS_AnnotationReloadTask(
        RLANN_DS_AnnotationPtr ann):
    _ann(ann)
{
}


void
RLANN_DS_AnnotationReloadTask::run()
{
    bool reloadResult =_ann->reload();
    if (!reloadResult) {
        RLP_LOG_WARN("annotation not reloaded");
    }
}