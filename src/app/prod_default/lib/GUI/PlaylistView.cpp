
#include "RlpProd/GUI/PlaylistView.h"
#include "RlpCore/UTIL/Convert.h"

RLP_SETUP_LOGGER(prod, GUI, PlaylistView)


GUI_PlaylistView::GUI_PlaylistView(GUI_ItemBase* parent):
    GUI_GRID_View(parent),
    _highlightDrop(false)
{
    setFlag(QQuickItem::ItemAcceptsDrops);

    _deptSwitchButton = new GUI_IconButton("", this, 20, 4);
    _deptSwitchButton->connectToMenu();
    _deptSwitchButton->setText("Dept:");
    _deptSwitchButton->setOutlined(true);
    _deptSwitchButton->setSvgIconPath(":feather/chevron-down.svg", 20, true);
    _deptSwitchButton->setVisible(false);

    _toolbar->layout()->addItem(_deptSwitchButton);
}


void
GUI_PlaylistView::dragEnterEvent(QDragEnterEvent* event)
{
    GUI_GRID_View::dragEnterEvent(event);
    return;


    RLP_LOG_DEBUG(event)

    // event->setAccepted(true);

    if (event->mimeData()->hasFormat("text/filepath"))
    {
        _header->setVisible(false);
        _toolbar->setVisible(false);
        _scrollArea->setVisible(false);

        _highlightDrop = true;
        update();
    }
}


void
GUI_PlaylistView::dragLeaveEvent(QDragLeaveEvent* event)
{
    RLP_LOG_DEBUG(event)
    _header->setVisible(true);
    _toolbar->setVisible(true);
    _scrollArea->setVisible(true);

    _highlightDrop = false;
    update();
}


void
GUI_PlaylistView::dropEvent(QDropEvent *event)
{
    RLP_LOG_DEBUG(event)

    _header->setVisible(true);
    _toolbar->setVisible(true);
    _scrollArea->setVisible(true);

    _highlightDrop = false;

    update();

    QString pathInfoStr;
    if (event->mimeData()->hasFormat("text/filepath"))
    {
        pathInfoStr = QString(event->mimeData()->data("text/filepath"));
        QVariantMap pathInfo = UTIL_Convert::fromJsonString(pathInfoStr);
        // RLP_LOG_DEBUG("GOT PATH:" << pathInfo)
        emit requestAddToPlaylist(pathInfo);
    } 


}


void
GUI_PlaylistView::onParentSizeChanged(qreal width, qreal height)
{
    GUI_GRID_View::onParentSizeChanged(width, height);
    // RLP_LOG_DEBUG(width << height << _toolbar->width())

    // _toolbar->setWidth(width);
    // _toolbar->colSelector()->setPos(width - 100, 0);
}


void
GUI_PlaylistView::paint(GUI_Painter* painter)
{
    if (_highlightDrop)
    {

        QPen p(Qt::white);
        p.setWidth(5);
        painter->setPen(p);
        painter->setBrush(QBrush(QColor(80, 80, 120)));

        QRectF r(5, 3, width() - 15, height() - 40);
        painter->drawRoundedRect(r, 10, 10);

        QFont f = painter->font();
        f.setPixelSize(30);
        painter->setFont(f);
        painter->setPen(p);
        painter->drawText(20, 40, "+ Media");
    }
}