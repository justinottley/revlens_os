#ifndef CORE_SEQ_FRAMESEQUENCE_H
#define CORE_SEQ_FRAMESEQUENCE_H

#include "RlpCore/SEQ/Global.h"
#include "RlpCore/SEQ/SequenceBase.h"

typedef QPair<int, int> FrameChunk;


class CORE_SEQ_API SEQ_FrameSequence {
    
public:
    
    static QString calculateFrameDescription(QList<SEQ_ItemPtr> seqList);
    
};

#endif