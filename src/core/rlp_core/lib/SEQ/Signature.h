#ifndef  CORE_SEQ_SIGNATURE_H
#define  CORE_SEQ_SIGNATURE_H

#include "RlpCore/SEQ/Global.h"
#include "RlpCore/SEQ/Hint.h"

class SEQ_Signature;
typedef std::shared_ptr<SEQ_Signature> SEQ_SignaturePtr;


class CORE_SEQ_API SEQ_Signature {
    
public:
    RLP_DEFINE_LOGGER

    SEQ_Signature(QString& nonDigits, QVariantList& sigList);
    SEQ_Signature(QString& nonDigits, QVariantList& sigList, QString& pattern);
    
    static SEQ_SignaturePtr buildSignature(QString& rawItem);
    static SEQ_SignaturePtr buildSignature(QString& rawItem, QString& pattern);
    
    static QString DEFAULT_PATTERN;
    
public:
    
    QString& nonDigits() { return _nonDigits; }
    const QVariantList& sigList() { return _sigList; }
    
    bool equals(SEQ_SignaturePtr itemSig);
    
    /*
     * get the iteration index between two items - the iteration index
     * is the index of the start of a digit entry that is varying
     * between the two signatures
     */
    int findIterationIndex(SEQ_SignaturePtr itemSig);
    
    
    QVariantMap entryAt(int idx);
    QString toString();
    
    
private:
    
    QString _nonDigits;
    const QVariantList _sigList;
    QString _pattern;
    
};

#endif
