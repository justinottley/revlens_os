

#include <iostream>

#include "RlpEdb/DS/DataSource.h"
#include "RlpEdb/DS/FieldType.h"
#include "RlpEdb/DS/EntityType.h"

#include "RlpEdb/STOR_SQL/StorageManager.h"

int main(int argc, char** argv)
{
    // std::cout << "hello world" << std::endl;

    QStringList stringAllowed;
    stringAllowed.append("str");

    DS_FieldType tName("Name", stringAllowed, DS_FieldType::NVAL_SINGLE);

    QVariantMap fieldData;
    QVariant v;
    v.setValue(tName),
    fieldData.insert("name", v);
    
    EdbDs_EntityType et("Asset", fieldData);

    
    std::cout << et.name().toStdString().c_str() << std::endl;
    std::cout << et.field("name").displayName().toStdString().c_str() << std::endl;
    
    
    EdbDs_DataSource dsrc("rlmedia", "media_container");
    
    EdbStorSql_StorageManager mgr;
    dsrc.bindStorage(&mgr);
    
    std::cout << dsrc.name().toStdString().c_str() << std::endl;
    
    dsrc.set_update_date(100);
    dsrc.set_update_user("jottley");
    
    
    // dsrc.flush();
}