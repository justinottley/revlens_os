

#include "RlpRevlens/GUI/ViewerLoadOverlay.h"

#include "RlpGui/BASE/Style.h"
#include "RlpGui/BASE/SvgIcon.h"

#include "RlpRevlens/DS/Session.h"
#include "RlpRevlens/CNTRL/MainController.h"

RLP_SETUP_LOGGER(prod, GUI, ViewerLoadOverlay)

GUI_ViewerLoadOverlay::GUI_ViewerLoadOverlay(GUI_ItemBase* parent):
    GUI_ItemBase(parent)
{
    _loadImgOn = GUI_SvgIcon::loadSvgFromPath(":misc/open.svg", 64, Qt::white);
    _loadImgOff = GUI_SvgIcon::loadSvgFromPath(":misc/open.svg", 64, GUI_Style::BgLightGrey);

    _compareImgOn = GUI_SvgIcon::loadSvgFromPath(":misc/compare.svg", 64, Qt::white);
    _compareImgOff = GUI_SvgIcon::loadSvgFromPath(":misc/compare.svg", 64, GUI_Style::BgLightGrey);
}


void
GUI_ViewerLoadOverlay::setMousePos(QPointF pos)
{
    // RLP_LOG_DEBUG("")

    _mousePos = pos;
    update();
}


int
GUI_ViewerLoadOverlay::getNumMediaTracks()
{
    int numMediaTracks = 0;
    DS_SessionPtr sess = CNTRL_MainController::instance()->session();
    for (int i=0; i != sess->numTracks(); ++i)
    {
        DS_TrackPtr tr = sess->getTrackByIndex(i);
        
        if (tr->trackType() != 0)
            continue;

        numMediaTracks++;
    }

    return numMediaTracks;
}


int
GUI_ViewerLoadOverlay::requestedAction()
{
    if (_mousePos.x() < width() / 2)
        return 0; // load

    return 1; // compare
}


DS_TrackPtr
GUI_ViewerLoadOverlay::requestedTrack()
{
    DS_SessionPtr sess = CNTRL_MainController::instance()->session();

    int numMediaTracks = getNumMediaTracks();
    float trackRectHeight = (float)height() / (float)numMediaTracks;

    float y = 0;
    for (int i=0; i != sess->numTracks(); ++i)
    {
        DS_TrackPtr tr = sess->getTrackByIndex(i);
        if (tr->trackType() != 0)
            continue;

        if ((_mousePos.y() > y) && (_mousePos.y() < y + trackRectHeight))
        {
            return tr;
        }

        y += trackRectHeight;
    }

    return nullptr;
}


QRectF
GUI_ViewerLoadOverlay::boundingRect() const
{
    return QRectF(0, 0, width(), height());
}


void
GUI_ViewerLoadOverlay::paint(GUI_Painter* painter)
{

    // QPen p(GUI_Style::BgLightGrey);
    // p.setWidth(3);

    // painter->setPen(p);

    // painter->drawRect(boundingRect());

    DS_SessionPtr sess = CNTRL_MainController::instance()->session();

    // RLP_LOG_DEBUG("TOTAL TRACKS:" << sess->numTracks())

    int numMediaTracks = getNumMediaTracks();
    float trackRectHeight = (float)height() / (float)numMediaTracks;

    // RLP_LOG_DEBUG("Num media tracks:" << numMediaTracks << "Track Rect Height:" << trackRectHeight)

    painter->setPen(GUI_Style::BgLightGrey);
    painter->setBrush(QBrush(GUI_Style::BgMidGrey));

    painter->drawRect(boundingRect()); // whalf, y, whalf, trackRectHeight - 2);

    QFont f = painter->font();
    f.setPixelSize(20);
    painter->setFont(f);


    float y = 0;
    for (int i=0; i != sess->numTracks(); ++i)
    {
        DS_TrackPtr tr = sess->getTrackByIndex(i);
        if (tr->trackType() != 0)
            continue;

        QRectF trect(0, y, width(), trackRectHeight - 2);

        QString trackName = sess->getTrackByIndex(i)->name();

        QPen pw(Qt::white);
        pw.setWidth(5);
            
        if ((_mousePos.y() > y) && (_mousePos.y() < y + trackRectHeight))
        {
            painter->setPen(pw);
            painter->setBrush(QBrush(GUI_Style::BgMidGrey));

        } else {
            painter->setPen(Qt::black);
            painter->setBrush(QBrush(Qt::black));
        }

        int ypos = y + (trackRectHeight / 2) + 20;

        float wqtr = width() / 4;
        float whalf = width() / 2;

        QRectF lrect(4, y, whalf - 8, trackRectHeight - 2);
        QRectF rrect(whalf + 4, y, whalf - 8, trackRectHeight - 2);

        if ((_mousePos.x() < whalf) && ((_mousePos.y() > y) && (_mousePos.y() < y + trackRectHeight)))
        {
            painter->setPen(pw);
            painter->setBrush(QBrush(QColor(80, 80, 120)));

            painter->drawRoundedRect(lrect, 10, 10);

            painter->drawImage(wqtr - 30, ypos - 30, &_loadImgOn);

            painter->setPen(Qt::white);
            painter->drawText(wqtr - 25, ypos + 60, "Load");

        }

        else
        {
            painter->setPen(GUI_Style::BgLightGrey);
            painter->setBrush(QBrush(GUI_Style::BgMidGrey));

            painter->drawImage(wqtr - 30, ypos - 30, &_loadImgOff);
        }


        if ((_mousePos.x() > whalf) && ((_mousePos.y() > y) && (_mousePos.y() < y + trackRectHeight)))
        {
            painter->setPen(pw);
            painter->setBrush(QBrush(QColor(80, 80, 120)));

            painter->drawRoundedRect(rrect, 10, 10);

            painter->drawImage(wqtr - 30, ypos - 30, &_loadImgOff);
            painter->drawImage((wqtr * 3) - 20, ypos - 30, &_compareImgOn);

            painter->setPen(Qt::white);
            painter->drawText((wqtr * 3) - 30, ypos + 60, "Compare");
        }

        else
        {
            painter->drawImage((wqtr * 3) - 20, ypos - 30, &_compareImgOff);
        }


        QPen pt(Qt::white);

        // painter->save();

        painter->setPen(pt);
        // painter->drawText((width() / 2)- 60, ypos, trackName);
        painter->drawText(60, y + 60, trackName);

        // painter->restore();

        y += trackRectHeight;
    }
}