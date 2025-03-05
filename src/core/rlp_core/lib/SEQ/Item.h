
#ifndef CORE_SEQ_ITEM_H
#define CORE_SEQ_ITEM_H

#include "RlpCore/SEQ/Global.h"

#include "RlpCore/SEQ/Signature.h"
#include "RlpCore/SEQ/Template.h"
#include "RlpCore/SEQ/SequenceBase.h"

class SEQ_Item;
typedef std::shared_ptr<SEQ_Item> SEQ_ItemPtr;


class CORE_SEQ_API SEQ_Item {
    
public:
    
    SEQ_Item(QString& rawItem);
    
    static SEQ_ItemPtr makeItem(QString& rawItem);
    
    SEQ_Sequence* sequence() { return _sequence; }
    
    int f();
    
    void setSequence(SEQ_Sequence* sequence) { _sequence = sequence; }
    
    QString rawItem() { return _rawItem; }
    
    SEQ_SignaturePtr signature();
    SEQ_TemplatePtr itemTemplate();
    
    
    // TODO FIXME: SEQ_FileItem subclass?
    //
    QFileInfo fileInfo();
    QString fullPath();
    
    
private:
    
    // a pointer back to the sequence for item
    SEQ_Sequence* _sequence;
    
    // a cached iteration entry (based on sequence iteration index)
    // in other words, the varying digit string in the sequence
    QString _fEntry;
    int _fNum;
    
    // a cached item template
    SEQ_TemplatePtr _template;
    
    QString _rawItem;
    SEQ_SignaturePtr _signature;
    
    
    
    QString _dirname;
    
    // the size of the padding for the iter entry
    int _padding;
    
};

#endif
