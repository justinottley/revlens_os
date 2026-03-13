

#include "RlpEdb/GRAPH/Global.h"
#include "RlpEdb/GRAPH/Parser.h"

EdbGraph_Parser::EdbGraph_Parser()
{
}


EdbGraph_Path
EdbGraph_Parser::parsePath(QByteArray pathIn)
{
    QJsonDocument pdoc = QJsonDocument::fromJson(pathIn);
    QJsonArray root = pdoc.array();
    
    qInfo() << "parsePath: " << root.size();
    
    EdbGraph_Path path;
    
    // build node from input
    //
    for (int i=0; i != root.size(); ++i)
    {
        QJsonObject ninfo = root.at(i).toObject();
        
        QString entityType = ninfo.value("etype").toString();
        
        int nodeLabel = i;
        
        if (entityType.length() > 0) {
            
            qInfo() << entityType;
            
            EdbGraph_NodePtr n(new EdbGraph_Node(entityType));
            
            
            if (ninfo.contains("filters")) {
                
                QJsonArray filters = ninfo.value("filters").toArray();
                QVariantList filterList;
                for (int fi=0; fi != filters.size(); ++fi) {
                    
                    QJsonArray filter = filters.at(fi).toArray();
                    QVariantList fr = filter.toVariantList();
                    
                    //fr.append(filter.at(0).toString());
                    //fr.append(filter.at(1).toString());
                    //fr.append(filter.at(2).toVariant());
                    
                    filterList.append(QVariant(fr));
                }
                
                n->setFilters(filterList);
                qInfo() << filterList;
                
            }
            
            QSet<QString> rset;
            //rset.insert("uuid");
            
            if (ninfo.contains("return")) {
                
                QJsonArray ra = ninfo.value("return").toArray();
                
                for (int ri = 0; ri != ra.size(); ++ri) {
                    rset.insert(ra.at(ri).toString());
                }
                
                
            }
            
            QStringList ret = rset.values();
            n->setReturnFields(ret);
            
            if (ninfo.contains("edge")) {
                
                QJsonArray edgeInfo = ninfo.value("edge").toArray();
                n->setEdgeField(edgeInfo.at(0).toString());
                
                if (edgeInfo.size() == 2) {
                    n->setEdgeLabel(edgeInfo.at(1).toInt());
                }
                
            }
            
            if (ninfo.contains("label")) {
                
                qInfo() << "setting label";
                nodeLabel = ninfo.value("label").toInt();
                
            }
            
            n->setLabel(nodeLabel);
                
                
            // _edgeIndex.insert(nodeLabel, n);
                
            path.insert(nodeLabel, n);
            
        }
        
    }
    
    return path;
    
}

