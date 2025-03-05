
#include "RlpCore/UTIL/Convert.h"


int main( int argc, char **argv )
{
    QJsonObject result;

    QVariantMap mapIn;

    QVariantList ml;
    QVariantMap mm;
    mm.insert("test", "the");
    
    ml.push_back(mm);

    mapIn.insert("test", 1);
    mapIn.insert("tlist", ml);


    UTIL_Convert::toJsonObject(mapIn, &result);

    QJsonDocument doc(result);

    QByteArray serialized = doc.toJson(QJsonDocument::Indented);

    qInfo() << serialized.data(); // QString(serialized);

}