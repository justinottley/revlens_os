
#include <iostream>

#include "RlpCore/SEQ/Hint.h"

RLP_SETUP_CORE_LOGGER(SEQ, Hint)

SEQ_Hint::SEQ_Hint(QString name, QStringList patternList):
    _name(name),
    _patternList(patternList)
{
}


SEQ_Hint::SEQ_Hint(QString name):
    _name(name)
{
}


void
SEQ_Hint::initRegex()
{
    RLP_LOG_INFO(_name)
    
    QStringList::iterator it;
    for (it = _patternList.begin(); it != _patternList.end(); ++it) {
        
        QRegularExpression rx(*it);
        
        if (rx.isValid()) {
            _regexList.push_back(rx);
            
        } else {
            
            RLP_LOG_ERROR("invalid regex " << (*it))
            
        }
    }
}


SignatureEntryResult
SEQ_Hint::findIterationIndex(QString& rawItem)
{
    RLP_LOG_DEBUG("");
    
    SignatureEntryResult result{-1 , ""};
    return result;
}


// -----------


QList<QRegularExpression> SEQ_FrameSignatureHint::REGEX_LIST;

SEQ_FrameSignatureHint::SEQ_FrameSignatureHint():
    SEQ_Hint("FrameSignatureHint")
{
    // called in SEQ_Init::initAll();
    // initRegex();
}


void
SEQ_FrameSignatureHint::initRegex()
{
    RLP_LOG_INFO("");
    
    QStringList patternList;
    patternList.push_back(QString("(.+)\\.(\\d+)\\.(\\w+)"));
    patternList.push_back(QString("(.+)\\.(None)\\.(\\w+)"));
    
    QStringList::iterator it;
    for (it = patternList.begin(); it != patternList.end(); ++it) {
        
        QRegularExpression rx(*it);
        
        if (rx.isValid()) {
            SEQ_FrameSignatureHint::REGEX_LIST.push_back(rx);
            
        } else {
            
            RLP_LOG_INFO("invalid regex " << (*it))
            
        }
    }
}


SignatureEntryResult
SEQ_FrameSignatureHint::findIterationIndex(QString& rawItem)
{
    // RLP_LOG_INFO("SEQ_FrameSignatureHint::findIterationIndex()" << std::endl;
    
    SignatureEntryResult result{-1, ""};
    
    
    QList<QRegularExpression>::iterator it;
    
    for (it = SEQ_FrameSignatureHint::REGEX_LIST.begin();
         it != SEQ_FrameSignatureHint::REGEX_LIST.end();
         ++it) {
        
        QRegularExpressionMatch match = (*it).match(rawItem);
        if (match.isValid())
        {
            QStringList capturedTexts = match.capturedTexts();
            if (capturedTexts.length() == 4)
            {
                result.idx = capturedTexts[1].size() + 1; // plus 1 for the dot
                result.entry = capturedTexts[1];
                
                break;
            }
        }
    }
    
    return result;
}
