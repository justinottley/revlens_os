
#include "RlpCore/PATH/Path.h"

RLP_SETUP_LOGGER(core, PATH, Path)

PATH_Path::PATH_Path(QString pathIn)
{

}


void
PATH_Path::registerStyle(PATH_StyleBase* style)
{
    RLP_LOG_DEBUG("")
}


PATH_PathInfo
PATH_Path::detect(QString pathIn)
{
    RLP_LOG_DEBUG(pathIn)

    PATH_PathInfo p;
    return p;
}
