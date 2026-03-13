


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
    EdbStorSql_Connection conn(fileName, "dump_data");
    QSqlQuery q(conn.db());
    
    q.prepare(""
        "SELECT * FROM edb_entity"
    );
    
    if (q.exec()) {
        
        QSqlQuery fq(conn.db());
        
        // QSqlResult* result = q.result();
        while(q.next()) {
            
            qInfo() << q.value(0).toInt();
            
            qInfo() << "    e_uuid: " << q.value(1).toString();
            qInfo() << "    e_name: " << q.value(2).toString();
            qInfo() << "    e_metadata: " << q.value(3).toString();
            qInfo() << "    e_et_uuid: " << q.value(4).toString();
            qInfo() << "    e_create_date: " << q.value(5).toInt();
            qInfo() << "    e_create_user: " << q.value(6).toString();
            qInfo() << "    e_update_date: " << q.value(7).toInt();
            qInfo() << "    e_update_user: " << q.value(8).toString();
            
            fq.prepare(""
                "SELECT * FROM edb_field_value WHERE "
                "fv_e_uuid = :e_id"
            );
            
            fq.bindValue(":e_id", q.value(1));
            
            if (fq.exec()) {
                
                while (fq.next()) {
                    
                    
                    qInfo() << "";
                    qInfo() << "      FieldValue: " << fq.value(0).toInt();
                    qInfo() << "";
                    qInfo() << "        fv_uuid: " << fq.value(1).toString();
                    qInfo() << "        fv_name: " << fq.value(2).toString();
                    qInfo() << "        fv_metadata: " << fq.value(3).toString();
                    qInfo() << "        fv_history: " << fq.value(4).toString();
                    qInfo() << "        fv_e_uuid: " << fq.value(5).toString();
                    qInfo() << "        fv_et_uuid: " << fq.value(6).toString();
                    qInfo() << "        fv_f_uuid: " << fq.value(7).toString();
                    qInfo() << "        fv_create_date: " << fq.value(8).toInt();
                    qInfo() << "        fv_create_user: " << fq.value(9).toString();
                    qInfo() << "        fv_update_date: " << fq.value(10).toInt();
                    qInfo() << "        fv_update_user: " << fq.value(11).toString();
                    qInfo() << "        fv_type_name: " << fq.value(12).toString();
                    qInfo() << "        fv_v_int: " << fq.value(13).toInt();
                    qInfo() << "        fv_v_str: " << fq.value(14).toString();
                    qInfo() << "        fv_v_float: " << fq.value(15).toDouble();
                    qInfo() << "        fv_v_json: " << fq.value(16).toString();
                    qInfo() << "        fv_v_binary: " << fq.value(17).toByteArray();
                    qInfo() << "        fv_v_e_uuid: " << fq.value(18).toString();
                    qInfo() << "        fv_v_et_uuid: " << fq.value(19).toString();
                    qInfo() << "        fv_v_dts_uuid: " << fq.value(20).toString();
                    
                    
                }
                
            }
            
        }
        
    }


    qInfo() << "";
    qInfo() << "------------";
    qInfo() << "";

    QSqlQuery fqa(conn.db());
    fqa.prepare("SELECT * FROM edb_field_value");

    if (fqa.exec())
    {
        while(fqa.next())
        {
            qInfo() << "";
            qInfo() << "      FieldValue: " << fqa.value(0).toInt();
            qInfo() << "";
            qInfo() << "        fv_uuid: " << fqa.value(1).toString();
            qInfo() << "        fv_name: " << fqa.value(2).toString();
            qInfo() << "        fv_metadata: " << fqa.value(3).toString();
            qInfo() << "        fv_history: " << fqa.value(4).toString();
            qInfo() << "        fv_e_uuid: " << fqa.value(5).toString();
            qInfo() << "        fv_et_uuid: " << fqa.value(6).toString();
            qInfo() << "        fv_f_uuid: " << fqa.value(7).toString();
            qInfo() << "        fv_create_date: " << fqa.value(8).toInt();
            qInfo() << "        fv_create_user: " << fqa.value(9).toString();
            qInfo() << "        fv_update_date: " << fqa.value(10).toInt();
            qInfo() << "        fv_update_user: " << fqa.value(11).toString();
            qInfo() << "        fv_type_name: " << fqa.value(12).toString();
            qInfo() << "        fv_v_int: " << fqa.value(13).toInt();
            qInfo() << "        fv_v_str: " << fqa.value(14).toString();
            qInfo() << "        fv_v_float: " << fqa.value(15).toDouble();
            qInfo() << "        fv_v_json: " << fqa.value(16).toString();
            qInfo() << "        fv_v_binary: " << fqa.value(17).toByteArray();
            qInfo() << "        fv_v_e_uuid: " << fqa.value(18).toString();
            qInfo() << "        fv_v_et_uuid: " << fqa.value(19).toString();
            qInfo() << "        fv_v_dts_uuid: " << fqa.value(20).toString();
        }
    }
}