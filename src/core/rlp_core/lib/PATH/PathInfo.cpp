
#include "RlpCore/PATH/PathInfo.h"

RLP_SETUP_LOGGER(core, PATH, PathInfo)

PATH_PathInfo::PATH_PathInfo(QStringList anchorParts, QStringList pathParts):
    _anchorParts(anchorParts),
    _pathParts(pathParts)
{
}


PATH_PathInfo::PATH_PathInfo(QStringList pathParts):
    _pathParts(pathParts)
{    
}


PATH_PathInfo::PATH_PathInfo(PATH_PathInfo& pin):
    _anchorParts(pin.anchorParts()),
    _pathParts(pin.pathParts()),
    _properties(pin.properties())
{
    
}


PATH_PathInfo::PATH_PathInfo(const PATH_PathInfo& pin)
{
    PATH_PathInfo* cpin = const_cast<PATH_PathInfo*>(&pin);

    _anchorParts = cpin->anchorParts();
    _pathParts = cpin->pathParts();
    _properties = cpin->properties();
}
