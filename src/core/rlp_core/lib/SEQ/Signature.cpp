

#include "RlpCore/SEQ/Signature.h"
#include "RlpCore/SEQ/Sequence.h"

QString SEQ_Signature::DEFAULT_PATTERN = QString("((\\d+)|(None))");

RLP_SETUP_LOGGER(core, SEQ, Signature)

SEQ_Signature::SEQ_Signature(QString& nonDigits, QVariantList& sigList, QString& pattern):
    _nonDigits(nonDigits),
    _sigList(sigList),
    _pattern(pattern)
{
}


SEQ_Signature::SEQ_Signature(QString& nonDigits, QVariantList& sigList):
    _nonDigits(nonDigits),
    _sigList(sigList),
    _pattern(DEFAULT_PATTERN)
{
}


SEQ_SignaturePtr
SEQ_Signature::buildSignature(QString& rawItem)
{
    return buildSignature(rawItem, SEQ_Signature::DEFAULT_PATTERN);
}


SEQ_SignaturePtr
SEQ_Signature::buildSignature(QString& rawItem, QString& pattern)
{
    // RLP_LOG_DEBUG(rawItem << pattern)

    QRegularExpression rx(pattern);
    
    // std::cout << "Valid regex: " << rx.isValid() << std::endl;
    
    int currIdx = 0;
    int digitIdxOffset = 0;
    
    
    // QString substrItem = rawItem;
    
    // QVariantList sigList;
    // sigList.clear();
    
    // int count = 0;
    int pos = 0;
    int lastPos = 0;
    
    QVariantList sigList;
    QString nonDigits = "";
    
    QRegularExpressionMatch match = rx.match(rawItem, pos);

    // while ((pos = rx.indexIn(rawItem, pos)) != -1) {
    int maxPos = rawItem.size();

    while (match.hasMatch())
    {
        // ++count;
        // RLP_LOG_DEBUG("valid: pos:" << pos << "curridx:" << currIdx << "capturedlength:" << match.capturedLength() << "capturedstart:"<< match.capturedStart())
        // RLP_LOG_DEBUG(match)

        QString nonDigitPart = rawItem.mid(currIdx, (match.capturedStart() - currIdx));

        // QString nonDigitPart = rawItem.mid(currIdx, (pos - currIdx));
        // QString digitPart = rawItem.mid(pos, match.capturedLength());
        QString digitPart = rawItem.mid(match.capturedStart(), match.capturedLength());
        
        // RLP_LOG_DEBUG("nonDigit:" << nonDigitPart << "digit:" << digitPart)
        /*
        std::cout << " --- " << std::endl;
        std::cout << "currIdx: " << currIdx << " pos: " << pos << "  - nondigit part: " << nonDigitPart.toStdString().c_str() << std::endl;
        std::cout << "digit part: " << digitPart.toStdString().c_str() << std::endl;
        */
        
        nonDigits += nonDigitPart;
        
        
        QVariantMap sigInfo;
        QString sigEntry = match.captured(0); // rawItem.mid(pos, match.capturedLength());

        sigInfo.insert("idx", match.capturedStart());
        sigInfo.insert("entry", sigEntry);
        sigInfo.insert("num", sigEntry.toInt());
        
        sigList.push_back(sigInfo);
        
        
        // std::cout << "pos: " << pos << std::endl;
        // std::cout << "matchedLength: " << rx.matchedLength() << std::endl;
        
        
        // pos = nextPos;
        pos += match.capturedStart() + match.capturedLength();
        lastPos = pos;
        
        // currIdx += pos; // rx.matchedLength();
        currIdx += pos;
        match = rx.match(rawItem, pos);

        if (currIdx >= maxPos)
        {
            // RLP_LOG_WARN("currIdx:" << currIdx << "maxPos:" << maxPos)
            break;
        }

    }
    
    
    // std::cout << "end pos: " << lastPos << std::endl;
    // std::cout << "siglist length: " << sigList.length() << std::endl;
    
    
    // add remaining non digits
    //
    // RLP_LOG_DEBUG("Adding remaining non digits:" << rawItem.mid(lastPos))
    nonDigits += rawItem.mid(lastPos);
    
    return std::shared_ptr<SEQ_Signature>(new SEQ_Signature(nonDigits, sigList, pattern));
}


int
SEQ_Signature::findIterationIndex(SEQ_SignaturePtr inSig)
{
    int idxResult = -1;
    
    int sigLength = _sigList.length();
    const QVariantList inSigList = inSig->sigList();
    
    if ((sigLength != inSigList.length()) || (_nonDigits != inSig->nonDigits())) {
        
        // std::cout << "SEQ_Signature::findIterationIndex(): FALSE - "
        //           << " sig list length or nondigit mismatch" << std::endl;
        
        // NOTE: early return
        return idxResult;
    }
    
    int mismatchCount = 0; // only 1 allowed, if more than 1, signatures are not a match
    
    for (int i = 0; i != sigLength; ++i) {
        
        QVariantMap mySigData = _sigList.at(i).toMap();
        QVariantMap inSigData = inSigList.at(i).toMap();
        
        int mySigIdx = mySigData.value("idx").value<int>();
        QString mySigEntry = mySigData.value("entry").value<QString>();
        
        
        int inSigIdx = inSigData.value("idx").value<int>();
        QString inSigEntry = inSigData.value("entry").value<QString>();
        
        // std::cout << i << " " << mySigEntry.toStdString().c_str() << " VS " << inSigEntry.toStdString().c_str() << std::endl;
        // std::cout << i << " " << mySigIdx << " VS " << inSigIdx << std::endl;
        
        if ((mySigEntry != inSigEntry) && (mySigIdx == inSigIdx)) {
            
            mismatchCount++;
            
            // std::cout << "SETTING INTERATION INDEX: " << mySigIdx << std::endl;
            
            // This is going to end up returning the last match. Thats how
            // the python impl. currently works..
            //
            idxResult = mySigIdx;
            
        } else if ((mySigEntry != inSigEntry) && (mySigIdx != inSigIdx)) {
            
            mismatchCount++;
            
        }
        
        
        if (mismatchCount > 1) {
            
            RLP_LOG_DEBUG("mismatched: second idx/entry mismatch at entry" << i
                      << ":" << mySigIdx << mySigEntry << "VS"
                      << inSigIdx << " " << inSigEntry)
                      
            idxResult = -1;
            break;
        }
    }
    
    
    // std::cout << "mismatch count: " << mismatchCount <<  " idx result: " << idxResult << std::endl;
    
    return idxResult;
}


bool
SEQ_Signature::equals(SEQ_SignaturePtr inSig)
{
    return (findIterationIndex(inSig) != -1);
}


QVariantMap
SEQ_Signature::entryAt(int idx)
{
    QVariantMap result;
    
    QVariantList::ConstIterator it;
    for (it = _sigList.begin(); it != _sigList.end(); ++it) {
        
        QVariantMap iterInfo = (*it).toMap();
        int iterEntryIdx = iterInfo.value("idx").value<int>();
        if (iterEntryIdx == idx) {
            
            result = iterInfo;
            // result = iterInfo.value("entry").value<QString>();
            break;
        }
    }
    
    
    return result;
}


QString
SEQ_Signature::toString()
{
    QString result = _nonDigits;
    result += " ";
    
    QVariantList::ConstIterator it;
    for (it = _sigList.begin(); it != _sigList.end(); ++it) {
        
        QVariantMap sigData = (*it).toMap();
        // std::cout << sigData.value("idx").value<int>() << std::endl;
        
        result += "[ ";
        result += QString("%1").arg(sigData.value("idx").value<int>());
        result += " " + sigData.value("entry").value<QString>();
        result += " ] ";
        
    }
    
    return result;
}

