
#include <iostream>

#include "RlpCore/SEQ/Sequence.h"
#include "RlpCore/SEQ/FrameSequence.h"

RLP_SETUP_CORE_LOGGER(SEQ, Sequence)

SEQ_Sequence::SEQ_Sequence():
    _signature(nullptr),
    _iterIdx(-1)
{
    _itemList.clear();
    _itemMap.clear();
    
    SEQ_HintPtr frameSigHint = SEQ_FrameSignatureHintPtr(new SEQ_FrameSignatureHint());
    _hintList.push_back(frameSigHint);
}


SEQ_SequencePtr
SEQ_Sequence::makeSequence()
{
    return SEQ_SequencePtr(new SEQ_Sequence());
}


QString
SEQ_Sequence::itemTemplateStr()
{
    QString result;
    
    if (_itemList.size() > 0) {
        result = _itemList[0]->itemTemplate()->format();
    }
    
    return result;
}


SEQ_TemplatePtr
SEQ_Sequence::itemTemplate()
{
    SEQ_TemplatePtr result = nullptr;
    
    if (_itemList.size() > 0) {
        result = _itemList[0]->itemTemplate();
    }
    
    return result;
}


void
SEQ_Sequence::sortItems()
{
    // This sort has to be able to sort non-padded f-numbers, e.g., 1, 2, 10
    // in that order, and not 1, 10, 2.
    // So we need to do some additional work - use the integer converted f-num
    // as the sort key
    //
    
    _itemList.clear();
    
    if (_itemMap.contains(INT_MIN)) {
        SEQ_ItemPtr firstItem = _itemMap.take(INT_MIN);
        _itemMap.insert(firstItem->f(), firstItem);
    }
    
    
    // It turns out QMap is automatically sorted by key. So we just have
    // to iterate on the map and repopulate the list
    //
    QList<int> keys = _itemMap.keys();
    QList<int>::iterator it;
    
    for (it = keys.begin(); it != keys.end(); ++it) {
        
        // RLP_LOG_DEBUG("SEQ_Sequence::sortItems(): " << _itemMap.value(*it)->rawItem().toStdString().c_str() << " " << (*it) );
        _itemList.push_back(_itemMap.value(*it));
    }
    
    
    _fDesc = SEQ_FrameSequence::calculateFrameDescription(_itemList);
    
    // RLP_LOG_DEBUG("FRAME DESC: " << _fDesc.toStdString().c_str() );
}


int
SEQ_Sequence::match(SEQ_ItemPtr item)
{
    // RLP_LOG_DEBUG("SEQ_Sequence::match(): " << item->rawItem().toStdString().c_str() );
    
    int result = -1;
    
    if (_itemMap.size() == 0) {
        
        // Try to identify via hint first
        //
        
        HintListIterator it;
        for (it = _hintList.begin(); it != _hintList.end(); ++it) {
            
            // RLP_LOG_DEBUG("Attempting match with hint " << (*it)->name().toStdString().c_str() );
            
            QString rawItem = item->rawItem();
            SignatureEntryResult res = (*it)->findIterationIndex(rawItem);
            
            if (res.idx != -1) {
                
                // RLP_LOG_DEBUG("MATCHED with " ); // << (*it)->name().toStdString().c_str() );
                
                result = res.idx;
                
                _iterIdx = res.idx;
                _signature = item->signature();
            }
        }
        
        // is there "for ... else" in C++?
        //
        // force a match if the sequence is emtpy to bootstrap
        // a signature match when the next item is added
        //
        if (result == -1) {
            result = -2;
        }
        
        
    } else { 
        
        int iterIdx = _signature->findIterationIndex(item->signature());
        
        if ((iterIdx != -1) && ((_iterIdx == -1) || _iterIdx == -2)) {
            
            // if ((_signature != nullptr) &&
            //       (item->signature()->equals(_signature))) {
            
            // RLP_LOG_DEBUG("SIGNATURE MATCH!" );
            // RLP_LOG_DEBUG("no iteration index yet, setting iteration index to " << iterIdx );
            
            _iterIdx = iterIdx;
        }
        
        result = iterIdx;
    }
    
    
    /*
    if (_signature != nullptr) {
        
        RLP_LOG_DEBUG("ME: " << _signature->toString().toStdString().c_str() );
        RLP_LOG_DEBUG("IN: " << item->signature()->toString().toStdString().c_str() );
        
        RLP_LOG_DEBUG(item->signature()->equals(_signature) );
    }
    */
    
    
    return result;
}


int
SEQ_Sequence::match(QString& item)
{
    SEQ_ItemPtr i(new SEQ_Item(item));
    return match(i);
}

void
SEQ_Sequence::insert(SEQ_ItemPtr item)
{
    item->setSequence(this);
    
    // RLP_LOG_DEBUG("SEQ_Sequence::insert(): " << item->rawItem().toStdString().c_str() << " " << item->f() );
    
    // RLP_LOG_DEBUG("item f: " << item->f().toStdString().c_str() );
    // RLP_LOG_DEBUG("item f again: " << item->f().toStdString().c_str() );
    
    _itemMap.insert(item->f(), item);
    _itemList.push_back(item); // will be modified if sortItems() is called
}


void
SEQ_Sequence::insert(QString& item)
{
    SEQ_ItemPtr i = SEQ_ItemPtr(new SEQ_Item(item));
    insert(i);
}


bool
SEQ_Sequence::append(QString& item)
{
    SEQ_ItemPtr i = SEQ_ItemPtr(new SEQ_Item(item));
    return append(i);
}


bool
SEQ_Sequence::append(SEQ_ItemPtr item)
{
    bool result = false;
    
    int matchResult = match(item);
    
    //
    // RLP_LOG_DEBUG("SEQ_Sequence::append(): " 
    //          << item->rawItem().toStdString().c_str() 
    //          << " : "
    //          << matchResult 
    //          );
    //
    
    
    if (matchResult >= 0) {
        
        insert(item);
        
        result = true;
        
    } else if (matchResult == -2) {
        
        RLP_LOG_DEBUG("no hint found for item "
                  << item->rawItem()
                  << " registering sequence signature as item signature" );
        
        insert(item);
        
        _itemList.push_back(item);
        
        _iterIdx = -2;
        _signature = item->signature();
    }
    
    
    return result;
}


bool
SEQ_Sequence::append(QStringList& itemList, bool sort)
{
    QStringList::iterator it;
    
    bool finalResult = true;
    
    for (it = itemList.begin(); it != itemList.end(); ++it) {
        finalResult &= append(*it);
    }
    
    if (sort) {
        sortItems();
    }
    
    return finalResult;
}
