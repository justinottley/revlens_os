
#include "RlpExtrevlens/RLANN_DS/PickerBrush.h"

#include "RlpGui/BASE/Style.h"

RLP_SETUP_EXT_LOGGER(RLANN_DS, PickerBrush)

RLANN_DS_PickerBrush::RLANN_DS_PickerBrush():
    RLANN_DS_BrushBase("Picker"),
    _zoom(1.0)
{
}


void
RLANN_DS_PickerBrush::activate(DISP_GLView* view, RLANN_DS_DrawControllerBase* annCntrl)
{

    RLP_LOG_DEBUG("")

    if (_view != view)
    {
        connect(
            view,
            &DISP_GLView::grabImageReady,
            this,
            &RLANN_DS_PickerBrush::setImage
        );

        connect(
            view,
            &DISP_GLView::displayZoomChanged,
            this,
            &RLANN_DS_PickerBrush::onDisplayZoomChanged
        );

        RLP_LOG_DEBUG("Connected to" << view)
    }

    RLANN_DS_BrushBase::activate(view, annCntrl);

    view->setCursor(Qt::CrossCursor);
    view->grabImage();
}


void
RLANN_DS_PickerBrush::setupFrame(RLANN_DS_AnnotationPtr ann)
{
    RLP_LOG_DEBUG("")
    _view->grabImage();
}


void
RLANN_DS_PickerBrush::setImage(QImage img)
{
    RLP_LOG_DEBUG(img.width() << img.height())

    _image = img;

    // _image.save("/tmp/test.jpg");
}


void
RLANN_DS_PickerBrush::onDisplayZoomChanged(float zoom)
{
    // RLP_LOG_DEBUG(zoom)
    if (_zoom != zoom)
    {
        _zoom = zoom;
        _view->grabImage();
    }
    
}


void
RLANN_DS_PickerBrush::strokeMove(QPointF& point, QVariantMap extra)
{
    if (_view == nullptr)
    {
        return;
    }

    if (_playstate == 0) // playing
    {
        _posText = "";
        _col = Qt::black;
        return;
    }

    RLP_LOG_DEBUG("Image:" << _image.width() << _image.height() << "View:" << _view->width() << _view->height())

    if ((_image.width() + 4) != (int)_view->width())
    {
        RLP_LOG_DEBUG("resyncing image")
        _view->grabImage();
        return;
    }

    

    // if ((point.x() > 0) && (point.y() > 0))
    // {

    QPointF displayPoint = extra.value("event.pos").value<QPointF>();

    _pos = displayPoint;
    _imgPos = point;
    _col = _image.pixelColor((int)displayPoint.x(), (int)displayPoint.y());
    _posText = QString("x: %1 y: %2").arg((int)point.x()).arg((int)point.y());

    _view->update();
}


void
RLANN_DS_PickerBrush::strokeEnd()
{
    if (_view != nullptr)
    {
        _view->grabImage();
    }
}


QVariantMap
RLANN_DS_PickerBrush::settings()
{
    QVariantMap info;
    info.insert("brush.name", _name);
    info.insert("r", _col.red());
    info.insert("g", _col.green());
    info.insert("b", _col.blue());
    info.insert("pos.x", _pos.x());
    info.insert("pos.y", _pos.y());

    return info;
}


void
RLANN_DS_PickerBrush::onPlayStateChanged(int state)
{
    RLANN_DS_BrushBase::onPlayStateChanged(state);

    _view->grabImage();
}


void
RLANN_DS_PickerBrush::paintGLPostDraw(GUI_Painter* painter, const QVariantMap& mdata)
{
    // RLP_LOG_DEBUG("")
    if (_playstate == 0) // playing
    {
        return;
    }


    QPen bp(GUI_Style::BgMidGrey);
    QBrush bb(GUI_Style::BgMidGrey);

    painter->setPen(Qt::black);
    painter->setBrush(bb);

    int wwidth = mdata.value("window.width").toInt();
    int wheight = mdata.value("window.height").toInt();

    // Clipping
    //
    int xpos = _pos.x();
    if (xpos + 140 > wwidth)
    {
        xpos = _pos.x() - 140;
    }

    if (xpos < 5)
    {
        xpos = 5;
    }

    int ypos = _pos.y() - 50;
    if (ypos < 5)
    {
        ypos = 5;
    }

    if (ypos + 110 > wheight)
    {
        ypos = wheight - 110;
    }

    // RLP_LOG_DEBUG(ypos << " -- " << _pos)
    // RLP_LOG_DEBUG(wwidth << wheight)
    // RLP_LOG_DEBUG("")

    painter->drawRect(
        xpos + 20,
        ypos,
        100,
        80
    );

    painter->setPen(Qt::white);
    QFont f = painter->font();
    f.setPixelSize(f.pixelSize() - 3);
    painter->setFont(f);

    painter->drawText(
        xpos + 30,
        ypos + 20, // _pos.y() - 30,
        QString("R: %1").arg(_col.red())
    );

    painter->drawText(
        xpos + 30,
        ypos + 35, // _pos.y() - 15,
        QString("G: %1").arg(_col.green())
    );

    painter->drawText(
        xpos + 30,
        ypos + 50, // _pos.y(),
        QString("B: %1").arg(_col.blue())
    );

    painter->drawText(
        xpos + 30,
        ypos + 70, // _pos.y() + 20,
        _posText
    );

    QBrush b(_col);

    painter->setPen(Qt::black);
    painter->setBrush(b);
    painter->drawRect(
        xpos + 80,
        ypos + 10, // _pos.y() - 40,
        30,
        30
    );
}