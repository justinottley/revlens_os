

#include <iostream>

#include <QtCore/QDebug>

#include "RlpEdb/DS/Entity.h"

#include "RlpEdb/STOR_SQL/Connection.h"
#include "RlpEdb/STOR_SQL/Traverser.h"

int main(int argc, char** argv)
{
    
    EdbStorSql_Connection conn("test.db");
    EdbStorSql_Traverser tr(&conn);
    
    EdbDs_EntityType et("MediaHandle");
    
    EdbDs_Entity e(&et, "", "{hd7d7djdjdj}");
    QVariant eVal;
    eVal.setValue(e);
    
    // QVariantList f1 = {"codec_short", "is", "vp8"};
    QVariantList f2 = {"frame_height", "is", 1080}; 
    
    QVariantList fe = {"handle", "is", eVal};
    
    QVariantList fn = {"name", "is", "video_main"};
    
    QVariantList filters;
    
    //filters.append(QVariant(f2));
    filters.append(QVariant(fn));
    
    // filters.append(QVariant(fe));
    
    QVariantList result = tr.find(
        "VideoStream",
        filters,
        { "frame_width" }
    );
    
    qInfo() << result; // .values();
    
    // dsrc.flush();
}