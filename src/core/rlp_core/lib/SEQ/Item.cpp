
#include <iostream>
#include "RlpCore/SEQ/Item.h"
#include "RlpCore/SEQ/Sequence.h"

SEQ_Item::SEQ_Item(QString& rawItem):
    _sequence(nullptr),
    _fEntry(""),
    _fNum(INT_MIN),
    _template(nullptr),
    _rawItem(rawItem)
{
    
    // _fNum initialized to INT_MIN to potentially support negative frame
    // numbers eventually
    
    // Potentially can lazily create the signature, but is always
    // created at the moment
    
    _signature = SEQ_Signature::buildSignature(_rawItem);
}


SEQ_ItemPtr
SEQ_Item::makeItem(QString& rawItem)
{
    return SEQ_ItemPtr(new SEQ_Item(rawItem));
}


SEQ_SignaturePtr
SEQ_Item::signature()
{
    return _signature;
}


SEQ_TemplatePtr
SEQ_Item::itemTemplate()
{
    if (_template == nullptr) {
        
        // std::cout << "SEQ_Item::itemTemplate(): building template" << std::endl;
        
        int iterIdx = _sequence->iterIdx();
        QVariantMap sigEntry = _signature->entryAt(iterIdx);
        int flength = sigEntry.value("entry").value<QString>().length();
        
        QString templatePrefix = _rawItem.mid(0, iterIdx);
        QString templateSuffix = _rawItem.mid(iterIdx + flength, _rawItem.length());
        
        _template = SEQ_Template::makeTemplate(templatePrefix, templateSuffix);
    }
    
    return _template;
}


int
SEQ_Item::f()
{
    if ((_fEntry == "") && (_sequence != nullptr)) {
        
        int iterIdx = _sequence->iterIdx();
        if ((iterIdx != -1) && (iterIdx != -2)) {
            
            QVariantMap sigEntry = _signature->entryAt(iterIdx);
            
            _fEntry = sigEntry.value("entry").value<QString>();
            _fNum = sigEntry.value("num").value<int>();
            
            // std::cout << "SEQ_Item::f(): iteration index " << iterIdx << " got iterEntry: " << _fEntry.toStdString().c_str() << " " << _fNum << std::endl;
        }
    }
    
    return _fNum;
}


QFileInfo
SEQ_Item::fileInfo()
{
    QFileInfo f;
    if (_sequence != nullptr) {
        f = QFileInfo(QDir(_sequence->dirname()), _rawItem);
    }
    
    return f;
}


QString
SEQ_Item::fullPath()
{
    QString result;
    
    if (_sequence != nullptr) {
        result = _sequence->dirname() + "/" + _rawItem;
    }
    
    return result;
}