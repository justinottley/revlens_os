
#include "RlpCore/LOG/Logging.h"

#include "RlpEdb/MEDIA/Controller.h"

#include <QtCore/QByteArray>
#include <QtCore/QBuffer>
#include <QtGui/QImage>

int main(int argc, char** argv)
{
    QCoreApplication::setApplicationName("rlmedia_enc");

    LOG_Logging::init();

    LOG_Logging::getLogger("edb.EdbStorSql")->setLevel(40);
    LOG_Logging::getLogger("edb.EdbFs")->setLevel(20);
    LOG_Logging::getLogger("edb.EdbDs")->setLevel(20);
    LOG_Logging::getLogger("core")->setLevel(20);

    if (argc < 5)
    {
        qInfo() << "ERROR: invalid arguments";
        exit(1);
    }

    QString frameSeqPrefix = argv[1];
    QString startStr = argv[2];
    int start = startStr.toInt();
    QString endStr = argv[3];
    int end = endStr.toInt();

    QString outFilename = argv[4];

    int count = end - start + 1;

    qInfo() << "start:" << startStr << "end:" << endStr;

    QString destFilename = QString("/home/justinottley/dev/revlens_root/sp_revsix/%1").arg(outFilename); // '("%1.rlm").arg(frameSeqPrefix);

    qInfo() << "Writing to:" << destFilename;

    EdbMedia_Controller c(destFilename);
    EdbDs_Entity* vs = c.addVideoStream("main.full");

    vs->setFieldValue("codec", QString("jpg")); // raw
    vs->setFieldValue("channel_count", 3);
    vs->setFieldValue("channel_order", 1);
    vs->setFieldValue("pixel_size", 1);
    vs->setFieldValue("colourspace", 0);
    vs->setFieldValue("start_frame", 1);
    vs->setFieldValue("end_frame", end);
    vs->setFieldValue("frame_rate", 23.98);

    EdbDs_Entity* ts = c.addVideoStream("main.thumbnail");
    ts->setFieldValue("codec", QString("jpg"));
    ts->setFieldValue("channel_count", 3);
    ts->setFieldValue("channel_order", 1);
    ts->setFieldValue("pixel_size", 1);
    ts->setFieldValue("colourspace", 0);


    for (int f = 0; f != count; ++f)
    {
        int nf = start + f;
        QString fpad = QString("%1").arg(nf).rightJustified(4, '0');
        QString fname = QString("%1.%2.jpg").arg(frameSeqPrefix).arg(fpad);
        //QString rname = QString("/Users/justinottley/Movies/trailers/moana_2_resized/moana_2.%2.jpg").arg(fpad);
        qInfo() << fname;
        // qInfo() << rname;
        qInfo();


        // if (f == 800)
        //     break;


        QImage i(fname);

        if ((i.width() == 0) || (i.height() == 0))
        {
            qInfo() << "Invalid input image, aborting";
            exit(1);
        }


        // qInfo() << "width:" << i.width() << "height" << i.height();

        QImage ti = i.scaledToWidth(240, Qt::SmoothTransformation);
        QByteArray tibytes;
        QBuffer b(&tibytes);
        b.open(QIODevice::WriteOnly);

        ti.save(&b, "jpg");

        QImage si = i.scaledToWidth(960, Qt::SmoothTransformation);

        if (f == 0)
        {
            vs->setFieldValue("width", si.width());
            vs->setFieldValue("height", si.height());
        }

        QByteArray fdata((char*)si.bits(), si.sizeInBytes());

        QByteArray sibytes;
        QBuffer sb(&sibytes);
        sb.open(QIODevice::WriteOnly);
        si.save(&sb, "jpg");

        // si.save(rname, "jpg");


        // qInfo() << f << "main.full:" << sibytes.size() << "image:" << i.sizeInBytes();
        // qInfo() << f << "main.thumbnail" << tibytes.size();
        // qInfo() << "";

        c.writeFrame("main.full", nf, sibytes); // fdata
        c.writeFrame("main.thumbnail", nf, tibytes);

    }

    return 0;

}