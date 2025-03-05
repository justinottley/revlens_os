
#include "RlpCore/LOG/Logging.h"

#include "RlpCore/PATH/StyleBase.h"
#include "RlpCore/PATH/PosixStyle.h"

int
main(int argc, char** argv)
{
    LOG_Logging::init();

    PATH_PosixStyle p;
    PATH_PosixStyle p2("/usr/anim");

    QString pathIn("/project/bbb/test.txt");
    QString pathIn2("/usr/anim/project/bbb/test.txt");

    PATH_PathInfo pinfo = p.detect(pathIn);
    PATH_PathInfo pinfo2 = p2.detect(pathIn2);

    qInfo() << pinfo.anchorParts() << " " << pinfo.pathParts();
    qInfo() << pinfo2.anchorParts() << " " << pinfo2.pathParts();

    qInfo() << "--";

    qInfo() << p.format(pinfo);
    qInfo() << p2.format(pinfo);

    qInfo() << "---";

    qInfo() << pinfo2.pathParts();
    qInfo() << p.format(pinfo) << " " << p2.format(pinfo, PATH_StyleBase::StyleAnchor);
    qInfo() << p2.format(pinfo2);
    

}