


#include "RlpEdb/DS/DataSource.h"
#include "RlpEdb/DS/FieldType.h"
#include "RlpEdb/DS/EntityType.h"
#include "RlpEdb/DS/Entity.h"

#include "RlpEdb/STOR_SQL/Connection.h"


int main(int argc, char** argv)
{
    QString fileName = argv[1];
    
    // qInfo() << fileName;
    
    // EdbStorSql_StorageManager mgr(fileName);
    EdbStorSql_Connection conn(fileName, "dump_schema");
    QSqlQuery q(conn.db());
    
    q.prepare(""
        "SELECT * FROM edb_data_source"
    );
    
    if (q.exec()) {
        
        // QSqlResult* result = q.result();
        while(q.next()) {
            
            qInfo() << q.value(0).toInt();
            
            qInfo() << "    dts_uuid: " << q.value(1).toString();
            qInfo() << "    dts_name: " << q.value(2).toString();
            qInfo() << "    dts_type: " << q.value(3).toString();
            qInfo() << "    dts_metadata: " << q.value(4).toString();
            qInfo() << "    dts_schema: " << q.value(5).toString();
            qInfo() << "    dts_origin: " << q.value(6).toInt();
            qInfo() << "    dts_create_date: " << q.value(7).toInt();
            qInfo() << "    dts_create_user: " << q.value(8).toString();
            qInfo() << "    dts_update_date: " << q.value(9).toInt();
            qInfo() << "    dts_update_user " << q.value(10).toString();
            
            
            
        }
    }
    
    
    q.prepare(""
        "SELECT * FROM edb_entity_type"
    );
    
    if (q.exec()) {
        
        qInfo() << "";
        qInfo() << "EntityType";
        qInfo() << "";
        
        QSqlQuery fq(conn.db());
        
        while (q.next()) {
            
            qInfo() << q.value(0).toInt();
            
            qInfo() << "    et_uuid: " << q.value(1).toString();
            qInfo() << "    et_name: " << q.value(2).toString();
            qInfo() << "    et_metadata: " << q.value(3).toString();
            qInfo() << "    et_dts_uuid: " << q.value(4).toString();
            qInfo() << "    et_parent_uuid: " << q.value(5).toString();
            qInfo() << "    et_create_date: " << q.value(6).toInt();
            qInfo() << "    et_create_user: " << q.value(7).toString();
            qInfo() << "    et_update_date: " << q.value(8).toInt();
            qInfo() << "    et_update_user: " << q.value(9).toString();
            
            fq.prepare(""
                "SELECT * FROM edb_field_type WHERE "
                "ft_et_uuid = :et_id"
            );
            
            qInfo() << q.value(1);
            
            fq.bindValue(":et_id", q.value(1));
            
            if (fq.exec()) {
                
                while (fq.next()) {
                    
                    
                    qInfo() << "";
                    qInfo() << "      FieldType: " << fq.value(0).toInt();
                    qInfo() << "";
                    qInfo() << "        ft_uuid: " << fq.value(1).toString();
                    qInfo() << "        ft_name: " << fq.value(2).toString();
                    qInfo() << "        ft_display_name: " << fq.value(3).toString();
                    qInfo() << "        ft_type_name: " << fq.value(4).toString();
                    qInfo() << "        ft_valtype: " << fq.value(5).toInt();
                    qInfo() << "        ft_et_uuid: " << fq.value(6).toString();
                    qInfo() << "        ft_metadata: " << fq.value(7).toString();
                    qInfo() << "        ft_create_date: " << fq.value(8).toInt();
                    qInfo() << "        ft_create_user: " << fq.value(9).toString();
                    qInfo() << "        ft_update_date: " << fq.value(10).toInt();
                    qInfo() << "        ft_update_user: " << fq.value(11).toString();
                    
                    
                }
                
                
                
            }
            
        }
        
    }
}