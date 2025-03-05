

#include "RlpGui/GRID/CellFormatterImage.h"
#include "RlpGui/GRID/Cell.h"

#include "RlpGui/GRID/Row.h"
#include "RlpGui/GRID/View.h"

RLP_SETUP_LOGGER(gui, GUI_GRID, CellFormatterImage)

GUI_GRID_CellFormatterImage::GUI_GRID_CellFormatterImage(QString keyName):
    _keyName(keyName)
{
}


QImage
GUI_GRID_CellFormatterImage::updateImage(GUI_GRID_Cell* cell, QImage im)
{
    // RLP_LOG_DEBUG("scaling to " << cell->width())

    im = im.scaledToWidth(
        cell->width() - 10,
        Qt::SmoothTransformation
    );

    QVariant vw;
    vw.setValue(im);

    cell->insertValue("image", im);

    cell->setHeight(im.height() + 5);
    cell->row()->setItemHeight(im.height() + 5);
    cell->row()->view()->updateRows();

    return im;
}


void
GUI_GRID_CellFormatterImage::paintCell(GUI_GRID_Cell* cell, GUI_Painter* painter)
{
    if (!cell->hasValue(_keyName))
    {
        return;
    }

    
    // RLP_LOG_DEBUG(val)

    
    // int pos = props.value("xpos").toInt();
    // int width = props.value("width").toInt();
    // int height = 20;

    int xoffset = 5 + (cell->treeDepth() * 15);
    if (cell->hasChildren()) {
        xoffset += 15; //  + (cell->ltreeDepth() * 15);
    }

    QImage im;

    if (cell->hasValue("image"))
    {
        im = cell->value("image").value<QImage>();

        if (im.width() != (cell->width() - 10))
        {
            im = updateImage(cell, cell->value("image.original").value<QImage>());
        }
    }

    // TODO FIXME: not super clear
    // would be better to have "image" mode and "image_b64" mode
    // Here we entertain actual "image_b64" mode where the image data is base64 encoded
    //
    if (im.width() != (cell->width() - 10))
    {
        // base64 based image
        QString thumbData = cell->value(_keyName).toString();

        if (thumbData.size() == 0)
        {
            return;
        }

        QImage i;
        i.loadFromData(QByteArray::fromBase64(thumbData.toLatin1()));

        im = updateImage(cell, i);
    }


    painter->drawImage(xoffset, 5, &im);
}
