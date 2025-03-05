
#include "RlpCore/LOG/Logging.h"

#include "RlpEdb/MEDIA/Controller.h"

#include <QtCore/QByteArray>
#include <QtCore/QBuffer>
#include <QtGui/QImage>

#include <turbojpeg.h>

int main(int argc, char** argv)
{
    QCoreApplication::setApplicationName("jpg_read_test");

    LOG_Logging::init();

    LOG_Logging::getLogger("edb.EdbStorSql")->setLevel(40);
    LOG_Logging::getLogger("edb.EdbFs")->setLevel(20);
    LOG_Logging::getLogger("edb.EdbDs")->setLevel(20);
    LOG_Logging::getLogger("core")->setLevel(20);

    if (argc < 2)
    {
        qInfo() << "ERROR: invalid arguments";
        exit(1);
    }

    QString jpgIn = argv[1];
    // QString destFilename = argv[2]; // outFilename = argv[4];

    qInfo() << jpgIn;

    FILE* jpegFile = NULL;
    tjhandle tjInstance = NULL;
    size_t jpegSize, sampleSize;
    int width, height;
    unsigned char* jpegBuf = NULL;
    void* dstBuf = NULL;

    if ((tjInstance = tj3Init(TJINIT_DECOMPRESS)) == NULL)
    {
        qInfo() << "error initializing turbojpeg";
        return 1;
    }

    if ((jpegFile = fopen(argv[1], "rb")) == NULL)
    {
        qInfo() << "error opening file";
        return 1;
    }

    if (fseek(jpegFile, 0, SEEK_END) < 0 || ((jpegSize = ftell(jpegFile)) < 0) ||
        fseek(jpegFile, 0, SEEK_SET) < 0)
    {
        qInfo() << "determining input file size";
        return 1;
    }


    if ((jpegBuf = (unsigned char *)malloc(jpegSize)) == NULL)
    {
        qInfo() << "allocating JPEG buffer";
        return 1;
    }


    if (fread(jpegBuf, jpegSize, 1, jpegFile) < 1)
    {
        qInfo() << "reading input file";
        return 1;
    }


    fclose(jpegFile);  jpegFile = NULL;


    if (tj3DecompressHeader(tjInstance, jpegBuf, jpegSize) < 0)
    {
        qInfo() << "could not read jpeg header";
        return 1;
    }

    qInfo() << "OK";

    return 0;

}