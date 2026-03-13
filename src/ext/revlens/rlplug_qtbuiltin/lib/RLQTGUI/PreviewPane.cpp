
#include "RlpExtrevlens/RLQTGUI/PreviewPane.h"
#include "RlpCore/UTIL/Convert.h"

RLP_SETUP_LOGGER(extrevlens, RLQTGUI, PreviewPane)

RLQTGUI_PreviewPane::RLQTGUI_PreviewPane(GUI_ItemBase* parent):
    RLQTDISP_PreviewViewBase(parent)
{
    setFlag(QQuickItem::ItemAcceptsDrops);

    _cntrl = new RLQTCNTRL_PreviewController();
    _cntrl->getVideoMgr()->setDisplay(this);

    _loadOverlay = new GUI_ViewerLoadOverlay(this);
    _loadOverlay->setSession(_cntrl->session());
    _loadOverlay->setCompareEnabled(false);
    _loadOverlay->setVisible(false);

    _currPath = "";

    onParentSizeChanged(parent->width(), parent->height());
}


void
RLQTGUI_PreviewPane::mousePressEvent(QMouseEvent* event)
{
    RLP_LOG_DEBUG("")

    if (_cntrl->getPlaybackState() == DS_ControllerBase::PLAYSTATE_PLAYING)
    {
        _cntrl->pause();
    } else
    {
        _cntrl->play();
    }
}


void
RLQTGUI_PreviewPane::dragEnterEvent(QDragEnterEvent* event)
{
    RLP_LOG_DEBUG(event->mimeData()->formats())

    event->setAccepted(true);
    const QMimeData* md = event->mimeData();

    if (md->hasFormat("text/filename") ||
        md->hasFormat("text/plain") ||
        md->hasFormat("text/uri-list") ||
        md->hasFormat("text/json"))
    {
        // RLP_LOG_DEBUG("TURN ON OVERLAY")

        _loadOverlay->setVisible(true);
        _loadOverlay->update();
    }

}


void
RLQTGUI_PreviewPane::dragLeaveEvent(QDragLeaveEvent* event)
{
    _loadOverlay->setVisible(false);
}


void
RLQTGUI_PreviewPane::dragMoveEvent(QDragMoveEvent* event)
{
    GUI_ItemBase::dragMoveEvent(event);

    _loadOverlay->setMousePos(event->position());
}


void
RLQTGUI_PreviewPane::dropEvent(QDropEvent* event)
{
    _loadOverlay->setVisible(false);

    QString path = "";
    if (event->mimeData()->hasFormat("text/filename"))
    {
        path = QString(event->mimeData()->data("text/filename"));
        // RLP_LOG_DEBUG(path)

    } else if (event->mimeData()->hasFormat("text/plain"))
    {
        path = QString(event->mimeData()->data("text/plain"));

        path = path.replace("file://", "");
        path = path.replace("\r", "");
        path = path.replace("\n", "");
        path = path.replace("%20", " ");

    } else if (event->mimeData()->hasFormat("text/uri-list"))
    {
        path = QString(event->mimeData()->data("text/uri-list"));

        path = path.replace("file:///", "");
        path = path.replace("\r", "");
        path = path.replace("\n", "");
        path = path.replace("%20", " ");

    } else if (event->mimeData()->hasFormat("text/json"))
    {
        QVariantMap payload = UTIL_Convert::fromJsonString(event->mimeData()->data("text/json"));
        LOG_Logging::pprint(payload);
        
        if (payload.contains("data"))
        {
            QVariantMap data = payload.value("data").toMap();
            if (data.contains("sg_path_to_frames"))
            {
                path = data.value("sg_path_to_frames").toString();
            }
        }
    }


    if (path == "")
    {
        return;
    }

    RLP_LOG_DEBUG("Drop Path:" << path)
    _loading = true;

    QStringList pathParts = path.split("/");
    _currPath = pathParts.at(pathParts.size() - 1);

    update();

    // RLP_LOG_DEBUG(result)
    _cntrl->loadMediaItem(path);

}


void
RLQTGUI_PreviewPane::onParentSizeChanged(qreal width, qreal height)
{
    // RLP_LOG_DEBUG(width << height)

    setWidth(width);
    setHeight(height);

    _loadOverlay->setWidth(width - 10);
    _loadOverlay->setHeight(height - 25);
}


void
RLQTGUI_PreviewPane::paint(GUI_Painter* painter)
{

    int w = width();
    int h = height();

    if (_loading)
    {
        painter->drawText(
            (w / 2),
            (h / 2),
            QString("Loading..")
        );
    } else
    {
        if (_image->width() > 0)
        {
            int xpos = (w - _image->width()) / 2;
            int ypos = (h - _image->height()) / 2;

            painter->drawImage(xpos, ypos, _image);
        }
    }

    painter->drawText(10, 20, _currPath);
    painter->drawText(w - 50, h - 40,
        QString::number(_cntrl->getVideoMgr()->currentFrameNum())
    );
}