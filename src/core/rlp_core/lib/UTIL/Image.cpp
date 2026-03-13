

#include "RlpCore/UTIL/Image.h"


RLP_SETUP_LOGGER(core, UTIL, Image)


QString
UTIL_Image::s_toBase64(QImage p, QString format)
{
    QByteArray ba;
    QBuffer rbuf(&ba);
    rbuf.open(QIODevice::WriteOnly);
    p.save(&rbuf, format.toLocal8Bit(), -1);
    rbuf.close();

    QString result = QString(ba.toBase64());
    return result;
}


QImage
UTIL_Image::s_fromBase64(QString b64input)
{
    QImage p;
    p.loadFromData(QByteArray::fromBase64(b64input.toLatin1()));

    return p;
}


QImage
UTIL_Image::s_fitImage(int width, int height, QImage image)
{
    QImage result(width, height, QImage::Format_RGBA8888);
    result.fill(Qt::transparent);

    int iwidth = image.width();
    int iheight = image.height();

    float iaspect = (float)iwidth / (float)iheight;
    float raspect = (float)width / (float)height;

    qInfo() << "s_fitImage(): incoming res: " << iwidth << iheight;
    qInfo() << "s_fitImage(): incoming aspect:" << iaspect << "result aspect:" << raspect;

    QImage sresult;
    int xpos = 0;
    int ypos = 0;

    if ((iaspect < 1) && (raspect > 1))
    {
        sresult = image.scaledToHeight(height, Qt::SmoothTransformation);
        xpos = (width - sresult.width()) / 2;
    }
    else if ((iaspect > 1) && (raspect > 1))
    {
        if ((iaspect > raspect))
        {
            sresult = image.scaledToWidth(width, Qt::SmoothTransformation);
            ypos = (height - sresult.height()) / 2;

        } else
        {
            sresult = image.scaledToHeight(height, Qt::SmoothTransformation);
            xpos = (width - sresult.width()) / 2;

        }
    }

    qInfo() << "s_fitImage(): x offset:" << xpos << "y offset:" << ypos;

    for (int x=0; x != sresult.width(); ++x)
    {
        for (int y=0; y != sresult.height(); ++y)
        {
            result.setPixel(xpos + x, ypos + y, sresult.pixel(x, y));
        }
    }
    
    
    /*
    if (width < iwidth)
    {
        QImage sresult = image.scaledToWidth(width, Qt::SmoothTransformation);

        if (sresult.height() < height)
        {
            int ypos = (height - result.height()) / 2;

            qInfo() << "s_fitImage(): y offset:" << ypos;
            
            for (int x=0; x != sresult.width(); ++x)
            {
                for (int y=0; y != sresult.height(); ++y)
                {
                    result.setPixel(x, y + ypos, sresult.pixel(x, y));
                }
            }
        }

    } else if (height < iheight)
    {
        QImage sresult = image.scaledToHeight(height, Qt::SmoothTransformation);

        if (sresult.width() < width)
        {
            int xpos = (width - result.width()) / 2;
            qInfo() << "s_imageFit(): x offset:" << xpos;
            
            for (int x=0; x != sresult.width(); ++x)
            {
                for (int y=0; y != sresult.height(); ++y)
                {
                    result.setPixel(xpos + x, y, sresult.pixel(x, y));
                }
            }
        }

    }
    */

    result.save("/tmp/fit.png");

    qInfo() << "s_ImageFit(): saved";

    return result;

}