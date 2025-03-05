//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_GUI_TL2_TRACK_H
#define REVLENS_GUI_TL2_TRACK_H

#include "RlpRevlens/GUI_TL2/Global.h"

#include "RlpCore/UTIL/AppGlobals.h"
#include "RlpGui/MENU/MenuPane.h"
#include "RlpRevlens/DS/Track.h"

#include "RlpRevlens/GUI_TL2/Item.h"
#include "RlpRevlens/GUI_TL2/TrackButton.h"

class GUI_TL2_Track;
class GUI_TL2_View;
class GUI_TL2_TrackManager;

typedef std::shared_ptr<GUI_TL2_Track> GUI_TL2_TrackPtr;
typedef QMap<QUuid, GUI_TL2_Item*>::iterator ItemIterator;


class REVLENS_GUI_TL2_API GUI_TL2_TrackTimelineArea : public GUI_ItemBase {
// Using this to occlude the info area since clipregion is not working?
public:
    RLP_DEFINE_LOGGER

    GUI_TL2_TrackTimelineArea(GUI_TL2_Track* track);
    void refresh();
    QRectF boundingRect() const;
    void paint(GUI_Painter* painter);

private:
    GUI_TL2_Track* _track;

};


class REVLENS_GUI_TL2_API GUI_TL2_Track : public GUI_ItemBase {
    Q_OBJECT

signals:
    void itemSelected(QVariantMap info);
    void actionRequested(QVariantMap info);


public:
    RLP_DEFINE_LOGGER

    GUI_TL2_Track(GUI_TL2_TrackManager* trackManager, DS_TrackPtr strack, QString trackName = QString());

    virtual ~GUI_TL2_Track();

    virtual void initCurrSession();
    virtual qreal trackHeight() { return _heightDefault; }
    virtual void initGearMenu();

    virtual void addItem(GUI_TL2_Item* trackItem);
    virtual void addButton(QString name, GUI_ButtonBase* trackButton);

    virtual void setYPos(int ypos);

    virtual void hoverEnterEvent(QHoverEvent* event);
    virtual void hoverLeaveEvent(QHoverEvent* event);

    void onParentSizeChanged(qreal width, qreal height);

    virtual QRectF boundingRect() const;
    virtual void paint(GUI_Painter* painter);

    GUI_TL2_TrackTimelineArea* timelineArea() { return _timelineArea; }

public slots:

    void onGearMenuClicked();
    void onRenameRequested();
    void onDeleteRequested();
    void onClearRequested();
    void onRemoveItemRequested();
    void onTrackDataChanged(QString evtName, QString trackUuid, QVariantMap changeInfo);

public slots:
    GUI_TL2_View* view();

    QUuid uuid() { return _strack->uuid(); }
    // virtual int height() const { return _heightDefault; }

    GUI_TL2_TrackManager* trackManager() { return _trackManager; }

    virtual void clearItems();
    bool clearItem(QString uuidStr);

    QString name() { return _strack->name(); }
    virtual void setName(QString name) { _strack->setName(name); }

    QString owner() { return _strack->owner(); }
    virtual void setOwner(QString owner) { _strack->setOwner(owner); }
    
    virtual QString label() { return name(); }

    QString trackTypeName() { return _strack->trackTypeName(); }
    int trackType() { return _strack->trackType(); }

    QString appOwner();
    bool isRemote();
    
    bool isActive() { return _active; }
    void setActive(bool isActive);

    virtual bool isButtonsVisible();
    void setButtonsVisible(bool vis, bool optionalOnly=false);

    void onButtonHoverEnter();
    void onButtonHoverLeave();

    GUI_ButtonBase* getButtonByName(QString name);

    GUI_TL2_Item* getItemByUuid(QString uuidStr);

    QColor colBg() { return _colBg; }
    QBrush brushBg();
    QColor colFg() { return _colFg; }

    GUI_MenuPane* gearMenu() { return _gearMenu; }

    QMap<QUuid, GUI_TL2_Item*> items() { return _itemMap; }


// signals:
//    void yposChanged(int ypos);

protected:

    // GUI_TL2_View* _view;
    GUI_TL2_TrackManager* _trackManager;
    DS_TrackPtr _strack;

    qreal _heightDefault;

    bool _active;
    bool _visButtons;
    bool _inButtonHover;

    GUI_MenuPane* _gearMenu;

    GUI_TL2_TrackTimelineArea* _timelineArea;

    QMap<QUuid, GUI_TL2_Item*> _itemMap;
    QMap<qlonglong, QUuid> _itemFrameMap;
    QMap<QString, GUI_ButtonBase*> _buttonMap;
    
    QColor _colBg;
    QColor _colFg;
};



#endif