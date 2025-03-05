

#include "RlpExtrevlens/RLQTDISP/CompositeImageWindow.h"

RLP_SETUP_EXT_LOGGER(RLQTDISP, CompositeImageWindow)

RLQTDISP_CompositeImageWindow::RLQTDISP_CompositeImageWindow(
    GUI_ItemBase* parent,
    QString displayUuidStr,
    int fbufWidth,
    int fbufHeight,
    QVariantMap* metadata):
        GUI_ItemBase(parent),
        _displayUuidStr(displayUuidStr),
        _fbufWidth(fbufWidth),
        _fbufHeight(fbufHeight),
        _windowWidth(0),
        _windowHeight(0),
        _displayZoom(1.0),
        _inHover(false),
        _enabled(true),
        _metadata(metadata)
{
    setAcceptHoverEvents(true);
}


void
RLQTDISP_CompositeImageWindow::hoverEnterEvent(QHoverEvent* event)
{
    // RLP_LOG_DEBUG("")

    _inHover = true;
    update();
}


void
RLQTDISP_CompositeImageWindow::hoverLeaveEvent(QHoverEvent* event)
{
    // RLP_LOG_DEBUG("")

    _inHover = false;
    update();
}


void
RLQTDISP_CompositeImageWindow::refresh(int nwidth, int nheight, float displayZoom, QVariantMap* fbufMetadata)
{
    //
    // This is assuming a x-oriented layout.
    // The algorithm here is a reproduction of what happens in opengl
    //
    qreal layoutScaleX = fbufMetadata->value("layout_scale.x").value<qreal>();
    qreal layoutScaleY = fbufMetadata->value("layout_scale.y").value<qreal>();

    qreal displayPanX = fbufMetadata->value("display_pan.x").value<qreal>();

    if ((layoutScaleX) == 1)
    {
        return;
    }


    _windowWidth = nwidth * layoutScaleX;
    _windowHeight = nheight;

    // RLP_LOG_DEBUG("")
    // RLP_LOG_DEBUG("fbuf:" << _fbufWidth << _fbufHeight)
    // RLP_LOG_DEBUG(nwidth << nheight << displayZoom)
    // RLP_LOG_DEBUG("layout:" << layoutScaleX << layoutScaleY)
    // RLP_LOG_DEBUG("pan:" << displayPanX)   
    // RLP_LOG_DEBUG("window:" << _windowWidth << _windowHeight)

    _displayZoom = displayZoom; // not used; broken for composites
    
    qreal maxHeight = nheight * layoutScaleY;
    qreal imageScale = (float)_windowWidth / (float)_fbufWidth;
    
    if (_fbufWidth > _windowWidth)
    {
        _nDestWidth = _windowWidth;
    }

    _nDestHeight = _fbufHeight * imageScale;

    if (_nDestHeight > maxHeight)
    {
        _nDestHeight = maxHeight;

        // rescale width to new height
        //
        imageScale = (float)_nDestHeight / (float)_fbufHeight;
        _nDestWidth = _fbufWidth * imageScale;
    }

    qreal xOffset = nwidth * displayPanX;

    _nLeft = xOffset + ((_windowWidth - _nDestWidth) / 2.0);
    _nTop = ((float)(_windowHeight - _nDestHeight)) / 2.0;

    // RLP_LOG_DEBUG("pos:" << _nLeft << _nTop)

    setWidth(_nDestWidth);
    setHeight(_nDestHeight);
    setPos(_nLeft, _nTop);  
    // RLP_LOG_DEBUG("layoutscale:" << layoutScaleX << "display_pan:" << displayPanX << "displayzoom:" << displayZoom)
    // RLP_LOG_DEBUG("nLeft:" << _nLeft << "xOffset:" << xOffset << "imageScale:" << imageScale)
    // RLP_LOG_DEBUG("Window incoming:" << width << height)
    // RLP_LOG_DEBUG("windowWidth:" << _windowWidth << _windowHeight << "image:" << _width << _height << "pos:" << _nLeft << _nTop << displayZoom)
    // RLP_LOG_DEBUG("nDest:" << _nDestWidth << _nDestHeight)

}



QRectF
RLQTDISP_CompositeImageWindow::boundingRect() const
{
    return QRectF(2, 2, width() - 4, height() - 4);
}


void
RLQTDISP_CompositeImageWindow::paint(GUI_Painter* painter)
{
    if (!_enabled)
    {
        return;
    }

    if (!_inHover)
    {
        return;
    }

    QPen p(QColor(120, 120, 120));
    p.setWidth(5);

    QBrush b(Qt::transparent);

    painter->setPen(p);
    painter->setBrush(b);
    painter->drawRect(boundingRect());

    // drop shadow
    painter->setPen(QColor(20, 20, 20));
    painter->drawText(
        21,
        height() - 9,
        _metadata->value("media.name").toString()
    );

    painter->setPen(QColor(230, 230, 230));
    painter->drawText(
        20,
        height() - 10,
        _metadata->value("media.name").toString()
    );
}
