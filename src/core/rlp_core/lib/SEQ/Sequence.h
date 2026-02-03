#ifndef CORE_SEQ_SEQUENCE_H
#define CORE_SEQ_SEQUENCE_H

#include <memory>

#include "RlpCore/LOG/Logging.h"
#include "RlpCore/SEQ/Global.h"
#include "RlpCore/SEQ/SequenceBase.h"
#include "RlpCore/SEQ/Item.h"
#include "RlpCore/SEQ/Hint.h"
#include "RlpCore/SEQ/Signature.h"
#include "RlpCore/SEQ/Template.h"


class CORE_SEQ_API SEQ_Sequence : public QObject {
    Q_OBJECT

public:
    
    RLP_DEFINE_LOGGER

    SEQ_Sequence();
    
    static SEQ_SequencePtr makeSequence();
    
    // SEQ_SequencePtr ptr() { return shared_from_this(); }
    
    virtual int match(QString& item);
    virtual int match(SEQ_ItemPtr item);
    
    // Low level insert
    //
    virtual void insert(QString& item);
    virtual void insert(SEQ_ItemPtr item);
    
    // High level append - runs match
    //
    virtual bool append(QString& item);
    virtual bool append(SEQ_ItemPtr item);
    virtual bool append(QStringList& itemList, bool sort=true);
    
    
    SEQ_ItemPtr at(int idx) { return _itemList.at(idx); }

public slots:

    int size() { return _itemList.size(); }
    void sortItems();


    /*
     * iteration index - index of start of digit iteration
     */
    int iterIdx() { return _iterIdx;}
    
    /*
     * "compressed" frame description - e.g., "1-10"
     */
    QString frange() { return _fDesc; }
    
    /*
     * template string with default fstring - e.g., "test.{f}.jpg"
     *
     * For convenience. Equivalent to itemTemplate()->format()
     */
    QString itemTemplateStr();
    
    /*
     * template object. can be (re)formatted to provide different formatting
     * for template string as may be required for different client uses.
     *
     * Will return nullptr if this sequence is empty
     */
    SEQ_TemplatePtr itemTemplate();
    
    
    QString dirname() { return _dirname; }
    void setDirname(QString dirname) { _dirname = dirname; }
    
    
protected:
    
    SEQ_SignaturePtr _signature;
    int _iterIdx;
    
    QList<SEQ_ItemPtr> _itemList;
    QMap<int, SEQ_ItemPtr> _itemMap;
    
    QList<SEQ_HintPtr> _hintList;
    
    // Cached frame description
    // TODO: May need to be refactored to a separate 
    QString _fDesc;
    
    // Directory where sequence lives.
    // TODO FIXME: doesn't really fit with this - should be in a FileSequence
    // subclass or something more related to files
    //
    QString _dirname;
};


Q_DECLARE_METATYPE(SEQ_SequencePtr);
Q_DECLARE_METATYPE(SEQ_Sequence);

#endif