
#include <iostream>

#include "RlpCore/SEQ/Sequence.h"
#include "RlpCore/SEQ/FrameSequence.h"


QString
SEQ_FrameSequence::calculateFrameDescription(QList<SEQ_ItemPtr> seqList)
{
    // std::cout << "SEQ_FrameSequence::calculateFrameDescription()" << std::endl;
    
    int nextF = INT_MIN;
    QList<FrameChunk> chunkList;
    
    for (int i=0; i != seqList.size(); ++i) {
        
        SEQ_ItemPtr item = seqList.at(i);
        
        if (item->f() != nextF) {
            
            FrameChunk newChunk(item->f(), item->f());
            
            chunkList.push_back(newChunk);
            nextF = item->f();
            
        } else {
            
            if (chunkList.size() > 0) {
                chunkList[chunkList.size() - 1].second = item->f();
            }
        }
        
        nextF++;
    }
    
    
    QString result;
    
    for (int x=0; x != chunkList.size(); ++x) {
        
        FrameChunk chunk = chunkList[x];
        
        if (result != "") {
            result += ",";
        }
        
        if (chunk.first == chunk.second) {
            result += QString("%1").arg(chunk.first);
            
        } else {
            result += QString("%1-%2").arg(chunk.first).arg(chunk.second);
        }
    }
    
    
    return result;
}

