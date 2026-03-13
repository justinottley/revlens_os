
#include "RlpExtrevlens/RLQTDISP/PreviewViewBase.h"

RLP_SETUP_LOGGER(extrevlens, RLQTDISP, PreviewViewBase)

RLQTDISP_PreviewViewBase::RLQTDISP_PreviewViewBase(GUI_ItemBase* parent):
    GUI_ItemBase(parent),
    _loading(false)
{
    _image = new QImage(); // for more efficient memory management with qnanopainter
}


void
RLQTDISP_PreviewViewBase::setImage(QImage image)
{
    _loading = false;

    image = image.scaled(width(), height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    _image->swap(image);
}