

#include <iostream>

#include <QtCore/QDebug>

#include "RlpEdb/DS/Entity.h"
#include "RlpEdb/GRAPH/Parser.h"
#include "RlpEdb/GRAPH/Node.h"

#include "RlpEdb/STOR_SQL/Traverser.h"

int main(int argc, char** argv)
{
    
    EdbStorSql_Connection conn("test.db");
    EdbStorSql_Traverser tr(&conn);
    
    // Format: list of dicts (TODO: spec lost..)
    //
    QString fileIn = "test_path.json";
    QFile testPath(fileIn);
    
    if (!testPath.open(QIODevice::ReadOnly)) {
        qInfo() << "error opening " << fileIn;
    }
    
    QByteArray pathIn = testPath.readAll();
    EdbGraph_Parser p;
    EdbGraph_Path path = p.parsePath(pathIn);
    
    tr.traverse(path);
}