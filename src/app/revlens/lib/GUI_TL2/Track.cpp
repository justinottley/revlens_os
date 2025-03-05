//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include "RlpCore/PY/Interp.h"

#include "RlpRevlens/GUI_TL2/Track.h"

#include "RlpRevlens/GUI_TL2/TrackManager.h"
#include "RlpRevlens/GUI_TL2/View.h"

#include "RlpRevlens/GUI_TL2/Item.h"

RLP_SETUP_LOGGER(revlens, GUI_TL2, TrackTimelineArea)
RLP_SETUP_LOGGER(revlens, GUI_TL2, Track)

GUI_TL2_TrackTimelineArea::GUI_TL2_TrackTimelineArea(GUI_TL2_Track* track):
    GUI_ItemBase(track),
    _track(track)
{
    setFlag(QQuickItem::ItemClipsChildrenToShape, true);

    // need this for the timeline area to paint over the button bar area
    setZValue(1);
}


void
GUI_TL2_TrackTimelineArea::refresh()
{
    // RLP_LOG_DEBUG("")

    setWidth(_track->view()->timelineWidth());
    setHeight(_track->view()->timelineHeight());// - 1);
    setPos(_track->view()->timelineXPosStart() - 1, 0); // 10: width of bookend drag button

}


QRectF
GUI_TL2_TrackTimelineArea::boundingRect() const
{
    return QRectF(0, 0, width(), height());
}


void
GUI_TL2_TrackTimelineArea::paint(GUI_Painter* painter)
{
    /*
    painter->setClipRect(
        _track->view()->timelineClipRect(),
        Qt::IntersectClip
    );
    */
    // painter->setPen(Qt::red);
    // painter->setBrush(Qt::red);
    
    //painter->setBrush(QBrush(_track->colBg()));
    //painter->setPen(QPen(_track->colFg()));

    // painter->drawRect(boundingRect());

}


GUI_TL2_Track::GUI_TL2_Track(GUI_TL2_TrackManager* trackManager, DS_TrackPtr strack, QString trackName):
    GUI_ItemBase(trackManager->view()),
    _trackManager(trackManager),
    _strack(strack),
    _heightDefault(25),
    _active(false),
    _visButtons(true),
    _inButtonHover(false),
    _colBg(QColor(20, 20, 20)),
    _colFg(QColor(50, 50, 50))
{
    setAcceptHoverEvents(true);
    setName(trackName);
    // setHeight(trackHeight());
    // setZValue(1);

    _timelineArea = new GUI_TL2_TrackTimelineArea(this);
    _gearMenu = new GUI_MenuPane(this);
    _gearMenu->setVisible(false);

    connect(
        strack.get(),
        &DS_Track::trackDataChanged,
        this,
        &GUI_TL2_Track::onTrackDataChanged
    );
}


GUI_TL2_Track::~GUI_TL2_Track()
{
    RLP_LOG_DEBUG("");

    setVisible(false);

    // TODO FIXME: reconcile with clearTracks() and onTrackDeleted()
    // clearItems();
    //_itemMap.clear();
}


void
GUI_TL2_Track::setActive(bool isActive)
{
    // RLP_LOG_DEBUG(name() << isActive)

    _active = isActive;
}


void
GUI_TL2_Track::hoverEnterEvent(QHoverEvent* event)
{
    // RLP_LOG_DEBUG("")

    _inHover = true;
    setButtonsVisible(true, /*optionalOnly=*/true);
}


void
GUI_TL2_Track::hoverLeaveEvent(QHoverEvent* event)
{
    // RLP_LOG_DEBUG(_inButtonHover);

    _inHover = false;
    if (!_inButtonHover)
    {
        setButtonsVisible(false, /*optionalOnly=*/true);
    }
    
}


void
GUI_TL2_Track::onButtonHoverEnter()
{
    // RLP_LOG_DEBUG("")

    _inButtonHover = true;
    setButtonsVisible(true, /*optionalOnly=*/true);
}


void
GUI_TL2_Track::onButtonHoverLeave()
{
    // RLP_LOG_DEBUG("")

    _inButtonHover = false;

    bool buttonsVisible = true;
    if (!_inHover)
    {
        buttonsVisible = false;
    }

    setButtonsVisible(buttonsVisible, /*optionalOnly=*/true);
}


void
GUI_TL2_Track::onParentSizeChanged(qreal nwidth, qreal nheight)
{
    // RLP_LOG_DEBUG(nwidth << nheight)

    setWidth(
        nwidth -
        GUI_TL2_VerticalScrollBar::AREA_WIDTH +
        1
    );

    setHeight(trackHeight());

    _timelineArea->refresh();
}


void
GUI_TL2_Track::initCurrSession()
{
    RLP_LOG_ERROR("Implement in subclass!")
}


void
GUI_TL2_Track::initGearMenu()
{
    GUI_MenuItem* removeItem = _gearMenu->addItem("Remove Item at Current Frame");
    connect(
        removeItem,
        SIGNAL(triggered()),
        this,
        SLOT(onRemoveItemRequested())
    );

    _gearMenu->addSeparator();


    GUI_MenuItem* renameTrack = _gearMenu->addItem("Rename..");

    connect(
        renameTrack,
        SIGNAL(triggered()),
        this,
        SLOT(onRenameRequested())
    );

    _gearMenu->addSeparator();

    GUI_MenuItem* clearTrack = _gearMenu->addItem("Clear");
    connect(
        clearTrack,
        SIGNAL(triggered()),
        this,
        SLOT(onClearRequested())
    );

    GUI_MenuItem* deleteTrack = _gearMenu->addItem("Delete");
    connect(
        deleteTrack,
        SIGNAL(triggered()),
        this,
        SLOT(onDeleteRequested())
    );

}


void
GUI_TL2_Track::onGearMenuClicked()
{
    
    QPointF cpos = QCursor::pos();
    GUI_ItemBase* gb = _buttonMap.value("gear_menu");

    QPointF spos = mapToScene(gb->pos());

    _gearMenu->setPos(gb->pos().x() + gb->width(), spos.y());
    _gearMenu->setVisible(!_gearMenu->isVisible());
}


void
GUI_TL2_Track::onRemoveItemRequested()
{
    RLP_LOG_DEBUG("");  

    QVariantList args;
    QVariant tr;
    tr.setValue(_strack.get());
    args.append(tr);

    PY_Interp::call("revlens.gui.timeline.request_remove_item_at_current_frame", args);
}


void
GUI_TL2_Track::onRenameRequested()
{
    RLP_LOG_DEBUG("")

    QVariantList args;
    QVariant tr;
    tr.setValue(this);
    args.append(tr);

    PY_Interp::call("revlens.gui.timeline.request_rename_track", args);
}


void
GUI_TL2_Track::onDeleteRequested()
{
    RLP_LOG_DEBUG("");

    QVariantList args;
    QVariant tr;
    tr.setValue(uuid().toString());
    args.append(tr);

    PY_Interp::call("revlens.gui.timeline.request_delete_track", args);
}


void
GUI_TL2_Track::onClearRequested()
{
    RLP_LOG_DEBUG("")

    QVariantList args;
    QVariant tr;
    tr.setValue(_strack.get());
    args.append(tr);

    PY_Interp::call("revlens.gui.timeline.request_clear_track", args);

}


void
GUI_TL2_Track::onTrackDataChanged(QString evtName, QString trackUuid, QVariantMap changeInfo)
{
    if (evtName == "set_name")
    {
        update();
    }

}


void
GUI_TL2_Track::clearItems()
{
    RLP_LOG_WARN(name())

    // Item clear skipped for stability / Windows compatibility");

    /*
     * TODO FIXME: DISABLED FOR WINDOWS??
     */

    ItemIterator it;
    for (it = _itemMap.begin(); it != _itemMap.end(); ++it)
    {
        
        (*it)->setParentItem(nullptr);
        // (*it)->clear();
        // view()->scene()->removeItem(*it);
    }


    _itemMap.clear();
    _itemFrameMap.clear();
}


bool
GUI_TL2_Track::clearItem(QString uuidStr)
{
    RLP_LOG_DEBUG(uuidStr);

    bool result = false;
    QUuid itemUuid(uuidStr);

    if (_itemMap.contains(itemUuid)) {
        GUI_TL2_Item* item = _itemMap.value(itemUuid);
        qlonglong itemStartFrame = item->startFrame();

        item->clear();

        _itemMap.remove(itemUuid);

        if (_itemFrameMap.contains(itemStartFrame)) {

            RLP_LOG_DEBUG("clearing " << itemUuid.toString() << " at " << itemStartFrame);
            _itemFrameMap.remove(itemStartFrame);

        }

        view()->scene()->removeItem(item);

        result = true;
    } else {

        RLP_LOG_ERROR("No item found: " << uuidStr);
    }

    view()->updateIfNecessary();

    return result;
}


bool
GUI_TL2_Track::isButtonsVisible()
{
    // RLP_LOG_DEBUG("height: " << height());

    return ((_visButtons) && (height() >= 40));
}


void
GUI_TL2_Track::setButtonsVisible(bool vis, bool optionalOnly)
{
    _visButtons = vis || optionalOnly;

    QMap<QString, GUI_ButtonBase*>::iterator it;
    for (it=_buttonMap.begin(); it != _buttonMap.end(); ++it)
    {
        if (!vis && optionalOnly && (it.key() == "visibility"))
        {
            // RLP_LOG_DEBUG("SKIPPING:" << it.value())
            continue;
        }

        it.value()->setVisible(vis);
    }

    _trackManager->refresh();
}


void
GUI_TL2_Track::addItem(GUI_TL2_Item* item)
{
    // RLP_LOG_DEBUG("");

    // item->setYPos(pos().y());

    // Crash for now for consistency check
    //
    Q_ASSERT(!(_itemMap.contains(item->uuid())));
    Q_ASSERT(!(_itemFrameMap.contains(item->startFrame())));

    _itemMap.insert(item->uuid(), item);
    _itemFrameMap.insert(item->startFrame(), item->uuid());

    // view()->scene()->addItem(item);
    view()->update();
}


void
GUI_TL2_Track::addButton(QString name, GUI_ButtonBase* button)
{
    // RLP_LOG_DEBUG("");

    button->setMetadata("act_name", name);
    button->setMetadata("track_uuid", uuid().toString());

    connect(
        button,
        &GUI_ButtonBase::buttonPressed,
        this,
        &GUI_TL2_Track::actionRequested
    );

    connect(
        button,
        &GUI_ButtonBase::hoverEnter,
        this,
        &GUI_TL2_Track::onButtonHoverEnter
    );

    connect(
        button,
        &GUI_ButtonBase::hoverLeave,
        this,
        &GUI_TL2_Track::onButtonHoverLeave
    );

    // Assuming all the buttons are the same size..
    int buttonXPos = 10 + (_buttonMap.size() * (button->size() + 4));

    // RLP_LOG_DEBUG(name << buttonXPos << button->size())

    button->setParentItem(this);
    button->setPos(buttonXPos, 20);
    _buttonMap.insert(name, button);
    // button->setPos()
}


GUI_ButtonBase*
GUI_TL2_Track::getButtonByName(QString name)
{
    GUI_ButtonBase* btn = nullptr;
    if (_buttonMap.contains(name)) {
        btn = _buttonMap.value(name);
    }

    return btn;
}


GUI_TL2_Item*
GUI_TL2_Track::getItemByUuid(QString uuidStr)
{
    QUuid itemUuid(uuidStr);
    if (_itemMap.contains(itemUuid)) {
        return _itemMap.value(itemUuid);
    }

    return nullptr;
}


GUI_TL2_View*
GUI_TL2_Track::view()
{
    return _trackManager->view();
}


QString
GUI_TL2_Track::appOwner()
{
    return UTIL_AppGlobals::instance()->globalsPtr()->value("username").toString();
}


bool
GUI_TL2_Track::isRemote()
{
    return (appOwner() != owner());
}


void
GUI_TL2_Track::setYPos(int ypos)
{
    // RLP_LOG_DEBUG(ypos)

    setPos(0, ypos);
    _timelineArea->refresh();
    
    ItemIterator it;
    for (it = _itemMap.begin(); it != _itemMap.end(); ++it) {
        (*it)->refresh(); // refresh(); // setYPos(ypos);
    }

    // emit yposChanged(ypos);
}


QRectF
GUI_TL2_Track::boundingRect() const
{
    return QRectF(0, 0, _trackManager->view()->width() - _trackManager->view()->timelineXPosRightPad(), height());
}



QBrush
GUI_TL2_Track::brushBg()
{
    QRectF br = boundingRect();

    QLinearGradient bgBrush = QLinearGradient(br.topLeft(), br.bottomLeft());// 0, 0, 0, 40);
    bgBrush.setColorAt(0.0, QColor(35, 35, 35)); // , 200));
    bgBrush.setColorAt(0.4, QColor(20, 20, 20)); // , 200));

    return bgBrush;
}


void
GUI_TL2_Track::paint(GUI_Painter* painter)
{
    // RLP_LOG_DEBUG("")

    // qInfo() << "TRACK CLIP REGION: " << _trackManager->view()->fullRegion();
    // qInfo() << "TRACK BOUNDING RECT: " << boundingRect();

    // painter->setClipRect(QRect(0, 0, 50, 50)); // _trackManager->view()->fullRegion());

    // qInfo() << "  " << painter->hasClipping();
    
    // QPen pen(_colFg); // QPen pen(Qt::red);
    // painter->setPen(pen);
    painter->setBrush(brushBg());
    painter->drawRect(boundingRect());

    painter->setPen(QPen(GUI_Style::TrackHeaderFg));
    painter->drawText(10, 15, label());
}