

#include "RlpCore/SEQ/Util.h"


RLP_SETUP_CORE_LOGGER(SEQ, Util)

SequenceList
SEQ_Util::getSequences(QString& inputDir)
{
    QDir dir(inputDir);
    
    QStringList files = dir.entryList(QDir::Files);
    SequenceList seqs = getSequencesFromList(files);

    for (int i = 0; i != seqs.size(); ++i)
    {
        seqs.at(i)->setDirname(inputDir);
    }

    return seqs;
}


QVariantList
SEQ_Util::getSequencesFromListAsVariant(QVariantList& stringList)
{
    QStringList ilist;
    for (int i = 0; i != stringList.size(); ++i)
    {
        ilist.append(stringList.at(i).toString());
    }

    SequenceList sresult = getSequencesFromList(ilist);
    QVariantList result;
    for (int si = 0; si != sresult.size(); ++si)
    {
        result.append(sresult.at(si)->itemTemplateStr());
    }

    return result;
}


SequenceList
SEQ_Util::getSequencesFromList(QStringList& inputList)
{
    // RLP_LOG_DEBUG(inputList);

    SequenceList result;
    SequenceList::iterator sit;

    QStringList::iterator it;

    for (it = inputList.begin(); it != inputList.end(); ++it) {
        
        QString fileName = (*it);
        SEQ_ItemPtr item = SEQ_Item::makeItem(fileName);
        
        // RLP_LOG_DEBUG(item->rawItem());
        
        bool seqFound = false;
        
        for (sit = result.begin(); sit != result.end(); ++sit) {
            
            SEQ_SequencePtr seq = (*sit);
            int idx = seq->match(item);
            
            if ((seq->iterIdx() == -1) || (seq->iterIdx() == idx)) {
                
                // RLP_LOG_DEBUG("found idx: " << idx );
                
                seq->insert(item);
                seqFound = true;
                break;
            }
        }
        
        if (!seqFound) {
            
            // RLP_LOG_DEBUG("creating new sequence for item " << item->rawItem().toStdString().c_str() );
            
            SEQ_SequencePtr seq = SEQ_Sequence::makeSequence();
            // seq->setDirname(inputDir);
            seq->append(item);
            
            result.push_back(seq);
            
        }
        
    }
    
    for (sit = result.begin(); sit != result.end(); ++sit) {
        (*sit)->sortItems();
    }
    
    return result;
}


QVariantList
SEQ_Util::getSequenceNames(QString& dir)
{
    SequenceList sl = SEQ_Util::getSequences(dir);

    QVariantList result;

    for (int i=0; i != sl.size(); ++i)
    {
        result.append(sl.at(i)->itemTemplateStr());
    }

    return result;
}


SEQ_SequencePtr
SEQ_Util::getSequenceFromTemplateString(QString& templatePathStr)
{
    RLP_LOG_DEBUG(templatePathStr);
    // RLP_LOG_DEBUG("SEQ_Util::getSequenceFromTemplateString(): " << fileInfo.dir().absolutePath().toStdString().c_str() );
    
    SEQ_SequencePtr result = nullptr;

    QFileInfo fileInfo(templatePathStr);
    QString inputDir = fileInfo.dir().absolutePath();

    if (fileInfo.exists())
    {
        RLP_LOG_DEBUG("Detected single file")

        // A single file
        //
        SEQ_SequencePtr seq = SEQ_Sequence::makeSequence();
        seq->setDirname(inputDir);

        QString fileName = fileInfo.fileName();
        SEQ_ItemPtr item = SEQ_Item::makeItem(fileName);
        seq->append(item);
            
        result = seq;

    } else 
    {
        
        QString templateStr = SEQ_Template::makeTemplate(
            fileInfo.fileName())->format();
        
        SequenceList seqList = SEQ_Util::getSequences(inputDir);
        
        
        
        RLP_LOG_DEBUG("found seq list for dir: " << seqList.size() );
        
        if (seqList.size() > 0) {
            
            SequenceList::iterator it;
            for (it = seqList.begin(); it != seqList.end(); ++it) {
                
                if ((*it)->itemTemplateStr() == templateStr) {
                    result = (*it);
                    break;
                }
            }
        }
    }

    return result;
}

