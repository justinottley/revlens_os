#ifndef CORE_SEQ_UTIL_H
#define CORE_SEQ_UTIL_H

#include "RlpCore/SEQ/Global.h"

#include "RlpCore/SEQ/Sequence.h"

typedef QList<SEQ_SequencePtr> SequenceList;

class CORE_SEQ_API SEQ_Util : public QObject {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    static QStringList TEMPLATE_SEQ_LIST;

public:
    
    static SequenceList getSequences(QString& dir);
    
    static SequenceList getSequencesFromList(QStringList& stringList);

    /*
     * Get a sequence from a fullpath + template string of files
     */
    static SEQ_SequencePtr getSequenceFromTemplateString(QString& templateStr);
    
public slots:

    static SEQ_Util* new_SEQ_Util() { return new SEQ_Util(); }
    QVariantList getSequenceNames(QString& dir);
    QVariantList getSequencesFromListAsVariant(QVariantList& stringList);

};

#endif