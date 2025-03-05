//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef EDB_DS_DATASOURCE_H
#define EDB_DS_DATASOURCE_H

#include "RlpEdb/DS/Global.h"
#include "RlpEdb/DS/EdbObjectBase.h"

class EDB_DS_API EdbDs_DataSource : public EdbDs_ObjectBase {
    Q_OBJECT

public:

    enum DtsOriginType {
        DTS_LOCAL,
        DTS_REMOTE
    };
    
    EdbDs_DataSource(QString name,
                  QString dataSourceType,
                  QString uuid="",
                  QString schema="",
                  DtsOriginType origin=DTS_LOCAL,
                  int version=1);

    EdbDs_DataSource();
    EdbDs_DataSource(const EdbDs_DataSource&);

    
public slots:

    QString type() { return _type; }
    void set_type(QString type) { _type = type; if (_autoSave) { save(); } }

    QString schema() { return _schema; }
    void set_schema(QString schema) { _schema = schema; if (_autoSave) { save(); } }

    DtsOriginType origin() { return _origin; }
    void set_origin(DtsOriginType origin) { _origin = origin; if (_autoSave) { save(); } }
    
    int version() { return _version; }
    void set_version(int version) { _version = version; if (_autoSave) { save(); } }
    
private:

    QString _type;
    QString _schema;
    DtsOriginType _origin;
    int _version;

};

Q_DECLARE_METATYPE(EdbDs_DataSource);


class EDB_DS_API EdbDs_DataSourceDecorator : public QObject {
    Q_OBJECT

public slots:

    EdbDs_DataSource*
    new_EdbDs_DataSource(QString name, QString dataSourceType) {
        return new EdbDs_DataSource(name, dataSourceType);
    }

};

#endif