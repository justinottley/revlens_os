

#include <iostream>

#include <QtCore/QDebug>

#include "RlpEdb/DS/DataSource.h"
#include "RlpEdb/DS/FieldType.h"
#include "RlpEdb/DS/EntityType.h"

#include "RlpEdb/STOR_SQL/StorageManager.h"

int main(int argc, char** argv)
{
    // std::cout << "hello world" << std::endl;
    EdbStorSql_StorageManager mgr("test.db");
    
    EdbDs_DataSource dsrc("rlmedia", "media_container");
    dsrc.bindStorage(&mgr);
    
    EdbDs_EntityType etVideoStream("VideoStream", &dsrc);
    
    
    etVideoStream.addField(
        "codec",
        "Codec",
        "str"
    );
    
    /*

    // DS_FieldType tName("Name", stringAllowed, DS_FieldType::NVAL_SINGLE);
    DS_FieldType tName(
        "name",
        "Name",
        DS_FieldType::FT_TYPE_STR);

    QVariantList fieldData;
    QVariant v;
    v.setValue(tName),
    fieldData.append(v);
    
    
    
    std::cout << et.name().toStdString().c_str() << std::endl;
    std::cout << et.field("name").display_name().toStdString().c_str() << std::endl;
    
    
    */
    
    
    
    // dsrc.flush();
}