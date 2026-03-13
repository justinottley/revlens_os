

#include "RlpEdb/DS/DataSource.h"
#include "RlpEdb/DS/FieldType.h"
#include "RlpEdb/DS/EntityType.h"
#include "RlpEdb/DS/Entity.h"


#include "RlpEdb/STOR_SQL/Connection.h"

int main(int argc, char** argv)
{
    QString fileName = argv[1];
    
    QString query = argv[2];

    // qInfo() << fileName;
    qInfo() << query;
    
    // EdbStorSql_StorageManager mgr(fileName);
    EdbStorSql_Connection conn(fileName);
    QSqlQuery q(conn.db());
    
    
    if (q.exec(query)) {
        qInfo() << "Success";

    } else {
        qInfo() << q.lastError().text();
    }
    
}