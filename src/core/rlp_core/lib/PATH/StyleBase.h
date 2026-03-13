#ifndef CORE_PATH_STYLEBASE_H
#define CORE_PATH_STYLEBASE_H

#include "RlpCore/PATH/Global.h"
#include "RlpCore/PATH/PathInfo.h"

class CORE_PATH_API PATH_StyleBase {

public:

    enum AnchorStyle {
        NoAnchor,
        PathInfoAnchor,
        StyleAnchor
    };

public:
    RLP_DEFINE_LOGGER

    PATH_StyleBase();

    virtual PATH_PathInfo detect(QString pathIn) = 0;
    virtual QString format(PATH_PathInfo p, AnchorStyle anchorStyle=PathInfoAnchor) = 0;

};

#endif