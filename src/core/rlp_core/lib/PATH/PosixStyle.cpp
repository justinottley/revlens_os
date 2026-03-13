
#include "RlpCore/PATH/PosixStyle.h"

RLP_SETUP_LOGGER(core, PATH, PosixStyle)

PATH_PosixStyle::PATH_PosixStyle(QString anchorPath):
    PATH_StyleBase(),
    _anchorPath(anchorPath)
{
    if ((anchorPath != POSIX_SEP) && (anchorPath.startsWith(POSIX_SEP)))
    {
        _anchorParts = anchorPath.split(POSIX_SEP);

    } else
    {
        _anchorParts.push_back(anchorPath);
    }
    
    RLP_LOG_DEBUG(_anchorParts << " " << _anchorPath)
}


PATH_PathInfo
PATH_PosixStyle::detect(QString pathIn)
{
    RLP_LOG_DEBUG(_anchorPath << " " << pathIn);

    PATH_PathInfo p;

    if (pathIn.startsWith(POSIX_SEP) &&
        pathIn.startsWith(_anchorPath))
    {

            RLP_LOG_DEBUG("OK")

            pathIn = pathIn.replace(0, _anchorPath.size(), "");

            p = PATH_PathInfo(
                _anchorParts,
                pathIn.split(POSIX_SEP, Qt::SkipEmptyParts)
            );
    }

    return p;
}


QString
PATH_PosixStyle::format(PATH_PathInfo p, PATH_StyleBase::AnchorStyle anchorStyle)
{
    QString result;
    if (anchorStyle == PATH_StyleBase::PathInfoAnchor)
    {
        result += p.anchorParts().join(POSIX_SEP);

    } else if (anchorStyle == PATH_StyleBase::StyleAnchor)
    {
        result += _anchorPath;
    }

    if (result.at(result.size() - 1) != POSIX_SEP)
    {
        result += POSIX_SEP;
    }
    
    result += p.pathParts().join(POSIX_SEP);

    return result;
}