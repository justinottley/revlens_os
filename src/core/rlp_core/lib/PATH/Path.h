
#ifndef CORE_PATH_PATH_H
#define CORE_PATH_PATH_H

#include "RlpCore/PATH/Global.h"
#include "RlpCore/PATH/StyleBase.h"
#include "RlpCore/PATH/PathInfo.h"

class CORE_PATH_API PATH_Path {

public:
    RLP_DEFINE_LOGGER

    PATH_Path(QString pathIn);

    static void registerStyle(PATH_StyleBase* style);

private:

    PATH_PathInfo detect(QString pathIn);

protected:
    static QList<PATH_StyleBase*> _styles;

};

#endif