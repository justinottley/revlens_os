
#ifndef REVLENS_AUDIO_READ_TASK_H
#define REVLENS_AUDIO_READ_TASK_H

#include "RlpRevlens/AUDIO/Global.h"

#include "RlpRevlens/DS/Node.h"

class REVLENS_AUDIO_API AUDIO_ReadAllTask : public QRunnable {

public:
    RLP_DEFINE_LOGGER

    AUDIO_ReadAllTask(DS_NodePtr node);

    void run();

private:
    DS_NodePtr _node;

};

#endif
