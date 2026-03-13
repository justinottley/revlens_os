#ifndef CORE_PATH_PATHINFO_H
#define CORE_PATH_PATHINFO_H

#include "RlpCore/PATH/Global.h"

class CORE_PATH_API PATH_PathInfo {

public:
    RLP_DEFINE_LOGGER

    PATH_PathInfo(
        QStringList anchorParts=QStringList(),
        QStringList pathParts=QStringList()
    );
    
    PATH_PathInfo(
        QStringList pathParts
    );
    
    PATH_PathInfo(PATH_PathInfo& pin);
    PATH_PathInfo(const PATH_PathInfo& pin);

    QStringList anchorParts() { return _anchorParts; }
    QStringList pathParts() { return _pathParts; }

    void insertProperty(QString key, QVariant val) { _properties.insert(key, val); }
    QVariant property(QString key) { return _properties.value(key); }

    QVariantMap properties() { return _properties; }


protected:

    QStringList _anchorParts;
    QStringList _pathParts;
    QVariantMap _properties;


};

#endif