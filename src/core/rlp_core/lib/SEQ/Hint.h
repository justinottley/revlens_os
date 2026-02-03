#ifndef CORE_SEQ_HINT_H
#define CORE_SEQ_HINT_H

#include "RlpCore/SEQ/Global.h"


class SEQ_Hint;
typedef std::shared_ptr<SEQ_Hint> SEQ_HintPtr;

typedef QList<SEQ_HintPtr>::iterator HintListIterator;

struct SignatureEntryResult {
    
    int idx;
    QString entry;
};


class CORE_SEQ_API SEQ_Hint {
    
public:
    
    RLP_DEFINE_LOGGER
    
    
    SEQ_Hint(QString name, QStringList patternList);
    SEQ_Hint(QString name);
    
    QString name() { return _name; }
    virtual SignatureEntryResult findIterationIndex(QString& rawItem);
    
    
protected:
    
    void initRegex();
    
protected:
    
    QString _name;
    QStringList _patternList;
    QList<QRegularExpression> _regexList;

};


class SEQ_FrameSignatureHint;
typedef std::shared_ptr<SEQ_FrameSignatureHint> SEQ_FrameSignatureHintPtr;

class CORE_SEQ_API SEQ_FrameSignatureHint : public SEQ_Hint {
    
public:
    
    static QList<QRegularExpression> REGEX_LIST;
    static void initRegex();
    
public:
    
    SEQ_FrameSignatureHint();
    
    SignatureEntryResult findIterationIndex(QString& rawItem);
    
};

#endif