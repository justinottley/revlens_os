
//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef CORE_DS_MODELDATAITEM_H
#define CORE_DS_MODELDATAITEM_H

#include "RlpCore/DS/Global.h"
#include "RlpCore/PY/Interp.h"

class CoreDs_ModelDataItem;
typedef std::shared_ptr<CoreDs_ModelDataItem> CoreDs_ModelDataItemPtr;

typedef QList<CoreDs_ModelDataItemPtr> ModelItemList;
typedef QMap<QString, CoreDs_ModelDataItemPtr> ModelItemMap;
typedef QMap<int, CoreDs_ModelDataItemPtr> ModelItemIntMap;


class CORE_DS_API CoreDs_ModelDataItem : public QObject {
    Q_OBJECT

signals:
    void populated();

public:
    RLP_DEFINE_LOGGER

    CoreDs_ModelDataItem();
    virtual ~CoreDs_ModelDataItem();

public slots:

    static CoreDs_ModelDataItem* new_CoreDs_ModelDataItem()
    {
        return new CoreDs_ModelDataItem();
    }

    static CoreDs_ModelDataItemPtr makeItem();

    void setPopulatePyCallback(py::object callback) { _pycallback = callback; }
    virtual int size();

    virtual QVariant value(QString key);
    void setValue(QString key, QVariant val) { _data.insert(key, val); }
    virtual QVariantMap values();
    
    virtual void populate();
    virtual void setPopulated();
    virtual bool contains(QString val);
    virtual bool containsItem(QString val);
    virtual void clear();

    virtual CoreDs_ModelDataItemPtr at(int i);

    virtual void append(CoreDs_ModelDataItemPtr i);
    virtual void push_back(CoreDs_ModelDataItemPtr i) {
        append(i);
    }

    virtual void insert(QString key, QVariant value);
    virtual void setMimeType(QString key, QString mtype);

    virtual bool hasMimeType(QString key);
    virtual QString mimeType(QString key);
    virtual void setMimeTypeKey(QString key) { _mimeTypeKey = key; }
    virtual void setMimeData(QVariant mimeData) { _mimeData = mimeData; }
    virtual QVariant mimeData() { return _mimeData; }

    QString mimeTypeKey() { return _mimeTypeKey; }

    virtual void insertItem(QString key, CoreDs_ModelDataItemPtr value);

    virtual CoreDs_ModelDataItemPtr item(QString key);

protected:
    
    ModelItemMap _items;
    ModelItemList _itemList;

    QVariantMap _data;
    QVariantMap _mimeTypes; // mapping from key to mimetype
    QString _mimeTypeKey; // key to use by default for mimetype
    QVariant _mimeData; // override mime data

    bool _populated;
    py::object _pycallback;

};

Q_DECLARE_METATYPE(CoreDs_ModelDataItemPtr)


#endif
