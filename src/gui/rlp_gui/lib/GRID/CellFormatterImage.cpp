

#include "RlpGui/GRID/CellFormatterImage.h"
#include "RlpGui/GRID/Cell.h"

#include "RlpGui/GRID/Row.h"
#include "RlpGui/GRID/View.h"

RLP_SETUP_LOGGER(gui, GUI_GRID, CellFormatterImageReadTask)
RLP_SETUP_LOGGER(gui, GUI_GRID, CellFormatterImage)

GUI_GRID_CellFormatterImageReadTask::GUI_GRID_CellFormatterImageReadTask(
    GUI_GRID_CellFormatterImage* fmt,
    GUI_GRID_Cell* cell,
    QString path
):
    _fmt(fmt),
    _cell(cell),
    _path(path)
{
}


void
GUI_GRID_CellFormatterImageReadTask::run()
{
    // RLP_LOG_DEBUG(_path)

    QImage i(_path);

    _fmt->emitImageReady(_cell, i, _path);
}



GUI_GRID_CellFormatterImage::GUI_GRID_CellFormatterImage(CellValueType cvt, QString keyName):
    _cellValueType(cvt),
    _keyName(keyName)
{

    _loadPool.setMaxThreadCount(12);

    connect(
        this,
        &GUI_GRID_CellFormatterImage::imageReady,
        this,
        &GUI_GRID_CellFormatterImage::onImageReady,
        Qt::AutoConnection
    );
}


void
GUI_GRID_CellFormatterImage::clearData()
{
    // RLP_LOG_DEBUG("waiting for done")

    _loadPool.clear();
    _loadPool.waitForDone();
}


QString
GUI_GRID_CellFormatterImage::getValue(GUI_GRID_Cell* cell)
{
    if (cell->row()->hasModelValue("code"))
    {
        return cell->row()->modelValue("code").toString();
    }

    return QString("- unknown -");
}


QImage
GUI_GRID_CellFormatterImage::updateImage(GUI_GRID_Cell* cell, QImage im)
{
    // RLP_LOG_DEBUG("scaling to " << cell->width())

    if (cell == nullptr)
    {
        return im;
    }

    if (cell->row() == nullptr)
    {
        return im;
    }

    QImage nim = im.scaledToWidth(
        cell->width() - 10,
        Qt::SmoothTransformation
    );

    QVariant vw;
    vw.setValue(nim);

    cell->insertValue("image", nim);

    cell->setHeight(nim.height() + 10);

    cell->row()->setItemHeight(nim.height() + 10);
    cell->row()->view()->updateRows();

    return nim;
}


void
GUI_GRID_CellFormatterImage::onImageReady(GUI_GRID_Cell* cell, QImage img, QString path)
{
    // RLP_LOG_DEBUG("")

    if (cell == nullptr)
    {
        return;
    }


    _imageCache.insert(path, img);

    cell->insertValue("image.original", img);
    cell->removeValue("image.loading");

    updateImage(cell, img);
}


void
GUI_GRID_CellFormatterImage::paintCell(GUI_GRID_Cell* cell, GUI_Painter* painter)
{
    if (!cell->hasValue(_keyName))
    {
        return;
    }

    // RLP_LOG_DEBUG(val)

    int xoffset = 5 + (cell->treeDepth() * 15);
    if (cell->hasChildren())
    {
        xoffset += 15;
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

    if (_cellValueType == FMT_VAL_B64)
    {
        if (im.width() != (cell->width() - 10))
        {
            QImage i;

            QVariant thumbRaw = cell->value(_keyName);

            if (thumbRaw.canConvert<QImage>())
            {
                i = thumbRaw.value<QImage>();

                cell->insertValue("image.original", thumbRaw);

            } else
            {
                // base64 based image
                QString thumbData = thumbRaw.toString();
                
                if (thumbData.size() == 0)
                {
                    painter->setPen(GUI_Style::FgGrey);
                    painter->drawText(xoffset, 20, "Loading..");
                    return;
                }

                i.loadFromData(QByteArray::fromBase64(thumbData.toLatin1()));

            }

            im = updateImage(cell, i);
        }

        painter->drawImage(xoffset, 5, &im);

    } else if (_cellValueType == FMT_VAL_PATH)
    {

        if (cell->hasValue("image"))
        {
            im = cell->value("image").value<QImage>();

            if (im.width() != (cell->width() - 10))
            {
                im = updateImage(cell, cell->value("image.original").value<QImage>());
            }

            painter->drawImage(xoffset, 5, &im);

        } else
        {
            QString imagePath = cell->value(_keyName).toString();

            if (_imageCache.contains(imagePath))
            {
                QImage i = _imageCache.value(imagePath);
                cell->insertValue("image.original", i);
                im = updateImage(cell, i);
                painter->drawImage(xoffset, 5, &im);
            }


            else if (!cell->hasValue("image.loading"))
            {
                // RLP_LOG_DEBUG(cell << "Loading" << imagePath)

                QRunnable* loadTask = new GUI_GRID_CellFormatterImageReadTask(this, cell, imagePath);
                cell->insertValue("image.loading", true);
                painter->setPen(GUI_Style::FgGrey);
                painter->drawText(xoffset, 15, "Loading..");

                _loadPool.start(loadTask);

            } else
            {
                painter->drawText(xoffset, 15, "---");
            }
        }
    }
}
