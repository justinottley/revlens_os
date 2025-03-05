
#include "RlpRevlens/GUI_TL2/TrackToggleButtonImage.h"

#include "RlpCore/UTIL/AppGlobals.h"
#include "RlpRevlens/GUI_TL2/Track.h"

RLP_SETUP_LOGGER(revlens, GUI_TL2, TrackToggleButtonImage)

GUI_TL2_TrackToggleButtonImage::GUI_TL2_TrackToggleButtonImage(GUI_ItemBase* parent, QString imagePathOn, QString imagePathOff):
    GUI_TL2_TrackToggleButton(parent)
{
    _colBgOn = QColor(20, 20, 20); // track->colBg();


    if (imagePathOff == "") {
        imagePathOff = imagePathOn;
    }
    
    _imagePathOn = imagePathOn;
    _imagePathOff = imagePathOff;

    _pixmapOn = QImage(_imagePathOn).scaledToHeight(size(), Qt::SmoothTransformation);
    _pixmapOff = QImage(_imagePathOff).scaledToHeight(size(), Qt::SmoothTransformation);

    // RLP_LOG_DEBUG(_imagePathOn << " " << _pixmapOn.size())
}



void
GUI_TL2_TrackToggleButtonImage::paintOn(GUI_Painter* painter)
{
    // RLP_LOG_DEBUG("")

    QBrush pbrush = painter->brush();
    pbrush.setColor(_colBgOn);

    QPen ppen = painter->pen();
    ppen.setColor(colPen());
    
    painter->setPen(ppen);
    painter->setBrush(pbrush);

    painter->drawRect(boundingRect());
    painter->drawImage(0, 0, &_pixmapOn);
}


void
GUI_TL2_TrackToggleButtonImage::paintOff(GUI_Painter* painter)
{
    // RLP_LOG_DEBUG("")

    // RLP_LOG_DEBUG(_imagePathOff)

    // TODO FIXME: was attempting to get transparency to work for png.
    // not working right now? not sure why
    //
    // painter->setPen(Qt::transparent);
    // painter->setBrush(QBrush(Qt::transparent));
    // painter->setCompositionMode(QPainter::CompositionMode_Source);

    painter->drawImage(0, 0, &_pixmapOff);
}