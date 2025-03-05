//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef EDB_DS_EDBOBJECTBASE_H
#define EDB_DS_EDBOBJECTBASE_H

#include <memory>

#include "RlpEdb/DS/Global.h"
#include "RlpEdb/DS/StorageHandleBase.h"

class EdbDs_ObjectBase;
typedef std::shared_ptr<EdbDs_ObjectBase>EdbDs_ObjectBasePtr;

class EdbDs_StorageManagerBase;

class EDB_DS_API EdbDs_ObjectBase : public QObject {
    Q_OBJECT
    
public:
    RLP_DEFINE_LOGGER
    
    EdbDs_ObjectBase(
        QString name="",
        QString uuid="",
        QVariantMap metadata=QVariantMap(),
        int createDate=0,
        QString createUser="",
        int updateDate=0,
        QString updateUser="",
        bool autoSave=true
    );
    
    virtual void bindStorage() {}
    virtual EdbDs_StorageHandleBase::BindResult bindStorage(EdbDs_StorageManagerBase* storage);
    
    virtual void save();
    virtual bool remove();    

    virtual void saveOne(QString methodName, int val);
    virtual void saveOne(QString methodName, QString val);
    virtual void saveOne(QString methodName, QJsonObject val);
    
public slots:

    virtual bool autoSave() { return _autoSave; }
    virtual void setAutoSave(bool autoSave) { _autoSave = autoSave; }

    /*
     * if object is not found, automatically create it in backend db
     * during bind()
     */
    virtual bool autoCreate() { return _autoCreate; }
    virtual void setAutoCreate(bool autoCreate) { _autoCreate = autoCreate; }

    QString name() { return _name; }
    void setName(QString name) { _name = name; if (_autoSave) { saveOne("name", name); }  }

    QString uuid() { return _uuid; }
    void set_uuid(QString uuid, bool doFlush) { _uuid = uuid; if (doFlush && _autoSave) { saveOne("uuid", uuid); } }
    
    QVariantMap metadata() { return _metadata; }
    void setMetadata(QVariantMap mdata); // { _metadata = mdata; if (_autoSave) { saveOne("metadata", metadataString()); } }
    
    QString metadataString();


    int createDate() { return _createDate; }
    void setCreateDate(int cdate) { _createDate = cdate; if (_autoSave) { saveOne("create_date", cdate); } }
    
    QString createUser() { return _createUser; }
    void setCreateUser(QString cuser) { _createUser = cuser; if (_autoSave) { saveOne("create_user", cuser); } }
    
    int updateDate() { return _updateDate; }
    void setUpdateDate(int udate) { _updateDate = udate; if (_autoSave) { saveOne("update_date", udate); } }

    QString updateUser() { return _updateUser; }
    void setUpdateUser(QString user) { _updateUser = user; if (_autoSave) { saveOne("update_user", user); } }

    EdbDs_StorageHandleBase* storageHandle() { return _sthandle; }

    virtual bool bound()
    {
        return ((_sthandle != nullptr) &&
                ((_bindResult == EdbDs_StorageHandleBase::BIND_NEW) ||
                 (_bindResult == EdbDs_StorageHandleBase::BIND_EXISTING)));
    }
    
protected:

    QString _name;
    QString _uuid;
    QVariantMap _metadata;
    
    
    int _createDate;
    QString _createUser;
    
    int _updateDate;
    QString _updateUser;
    
    EdbDs_StorageHandleBase* _sthandle;
    EdbDs_StorageHandleBase::BindResult _bindResult;

    bool _autoSave;
    bool _autoCreate;

};

// Q_DECLARE_METATYPE(EdbDs_StorageBase);

#endif