
#ifndef CORE_PATH_POSIX_H
#define CORE_PATH_POSIX_H

#include "RlpCore/PATH/Global.h"
#include "RlpCore/PATH/StyleBase.h"
#include "RlpCore/PATH/PathInfo.h"

class CORE_PATH_API PATH_PosixStyle : public PATH_StyleBase {

public:
    RLP_DEFINE_LOGGER

    PATH_PosixStyle(QString anchorPath=POSIX_SEP);

    PATH_PathInfo detect(QString pathIn);
    QString format(
        PATH_PathInfo p,
        PATH_StyleBase::AnchorStyle anchorStyle=PATH_StyleBase::PathInfoAnchor
    );

    void setAnchorPath(QString path);
    QStringList anchorParts() { return _anchorParts; }

protected:
    QStringList _anchorParts;
    QString _anchorPath;

};


#endif
