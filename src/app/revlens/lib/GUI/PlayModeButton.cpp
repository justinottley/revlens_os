
#include "RlpRevlens/GUI/PlayModeButton.h"

#include "RlpRevlens/DS/PlaybackMode.h"

#include "RlpRevlens/CNTRL/MainController.h"
#include "RlpRevlens/CNTRL/Video.h"

GUI_PlayModeButton::GUI_PlayModeButton(GUI_ItemBase* parent, int iconSize):
    GUI_IconButton("", parent, iconSize)
{
    setToolTipText("Play Mode");

    GUI_SvgIcon* playIconLoop = new GUI_SvgIcon(
        ":misc/replay.svg", this, iconSize, 0, GUI_Style::IconBg, /*changeColor=*/true);

    GUI_SvgIcon* playIconOnce = new GUI_SvgIcon(
        ":misc/repeat-once.svg", this, iconSize - 6, 0, GUI_Style::IconBg, /*changeColor=*/true);
    playIconOnce->setVisible(false);
    playIconOnce->setPos(3, 5);


    GUI_SvgIcon* playIconSwing = new GUI_SvgIcon(
        ":misc/transfer.svg", this, iconSize - 10, 0, GUI_Style::IconBg, /*changeColor=*/true);
    playIconSwing->setVisible(false);
    playIconSwing->setPos(3, 4);

    // playModeButton->setSvgIconPath(":misc/replay.svg", 26);
    setSvgIcon(playIconLoop);
    svgIcon()->setPos(0, 0);
    setWidth(28);

    GUI_MenuPane* mp = qobject_cast<GUI_MenuPane*>(menu());
    // mp->setExclusiveSelect(true);
    connect(
        mp,
        &GUI_MenuPane::menuItemSelected,
        this,
        &GUI_PlayModeButton::onPlayModeChanged
    );

    connect(
        this,
        &GUI_PlayModeButton::menuShown,
        this,
        &GUI_PlayModeButton::onPlayModeMenuShown
    );

    GUI_MenuItem* hi = mp->addItem("Playback");
    hi->setSelectable(false);

    QVariantMap mdl;
    QVariant vl;
    vl.setValue(playIconLoop);
    mdl.insert("icon", vl);
    mdl.insert("val", DS_PlaybackMode::LOOP);
    mdl.insert("type", "loop");
    
    QVariantMap mdo;
    QVariant vo;
    vo.setValue(playIconOnce);
    mdo.insert("icon", vo);
    mdo.insert("val", DS_PlaybackMode::NONE);
    mdo.insert("type", "loop");

    QVariantMap mdpp;
    QVariant vpp;
    vpp.setValue(playIconSwing);
    mdpp.insert("icon", vpp);
    mdpp.insert("val", DS_PlaybackMode::SWING);
    mdpp.insert("type", "loop");

    _lmLoop = mp->addItem("Loop", mdl, /*checkable=*/ true);
    _lmLoop->setChecked(true);

    _lmOnce = mp->addItem("Once", mdo, /*checkable=*/ true);
    _lmSwing = mp->addItem("Swing", mdpp, /*checkable=*/ true);

    QVariantMap mdf;
    mdf.insert("val", DS_PlaybackMode::FORWARD);
    mdf.insert("type", "dir");

    QVariantMap mdb;
    mdb.insert("val", DS_PlaybackMode::BACKWARD);
    mdb.insert("type", "dir");

    mp->addItem(""); // spacer


    QVariantMap mda;
    GUI_MenuItem* playdir = mp->addItem("Direction");
    playdir->setSelectable(false);

    _dirForward = mp->addItem("Forward", mdf, /*checkable=*/ true);
    _dirForward->setChecked(true);
    _dirBackward = mp->addItem("Backward", mdb, /*checkable=*/ true);
    _dirAuto = mp->addItem("Auto", mda, /*checkable=*/ true);
    _dirAuto->setItemEnabled(false);

}


void
GUI_PlayModeButton::syncMenu()
{
    CNTRL_Video* vmgr = CNTRL_MainController::instance()->getVideoManager();
    DS_PlaybackMode* pbm = vmgr->playbackMode();

    DS_PlaybackMode::LoopMode currLoopMode = pbm->getLoopMode();
    DS_PlaybackMode::DirectionMode currDir = pbm->getDirection();

    _dirForward->setItemEnabled(true);
    _dirBackward->setItemEnabled(true);
    _dirAuto->setChecked(false);

    if (currLoopMode == DS_PlaybackMode::LOOP)
    {
        _lmLoop->setChecked(true);
        _lmOnce->setChecked(false);
        _lmSwing->setChecked(false);
    }
    else if (currLoopMode == DS_PlaybackMode::NONE)
    {
        _lmLoop->setChecked(false);
        _lmOnce->setChecked(true);
        _lmSwing->setChecked(false);
    }
    else if (currLoopMode == DS_PlaybackMode::SWING)
    {
        _lmLoop->setChecked(false);
        _lmOnce->setChecked(false);
        _lmSwing->setChecked(true);

        _dirForward->setChecked(false);
        _dirForward->setItemEnabled(false);

        _dirBackward->setChecked(false);
        _dirBackward->setItemEnabled(false);

        _dirAuto->setChecked(true);
    }

    if (currLoopMode != DS_PlaybackMode::SWING)
    {
        _dirForward->setItemEnabled(true);
        _dirBackward->setItemEnabled(true);

        if (currDir == DS_PlaybackMode::FORWARD)
        {
            _dirForward->setChecked(true);
            _dirBackward->setChecked(false);

        } else if (currDir == DS_PlaybackMode::BACKWARD)
        {
            _dirForward->setChecked(false);
            _dirBackward->setChecked(true);
        }
    }


    _dirForward->update();
    _dirBackward->update();
    _dirAuto->update();
}


void
GUI_PlayModeButton::onPlayModeChanged(QVariantMap md)
{
    // RLP_LOG_DEBUG(md)

    QString text = md.value("text").toString();

    QVariantMap data = md.value("data").toMap();
    int val = data.value("val").toInt();
    QString valtype = data.value("type").toString();

    CNTRL_Video* vmgr = CNTRL_MainController::instance()->getVideoManager();

    if (valtype == "loop")
    {
        DS_PlaybackMode::LoopMode loopModeVal = (DS_PlaybackMode::LoopMode)val;
        vmgr->setLoopMode(loopModeVal);

        GUI_SvgIcon* icon = data.value("icon").value<GUI_SvgIcon*>();
        icon->setVisible(true);

        svgIcon()->setVisible(false);
        setSvgIcon(icon);
    }

    else if (valtype == "dir")
    {
        DS_PlaybackMode::DirectionMode dirModeVal = (DS_PlaybackMode::DirectionMode)val;
        vmgr->setDirectionMode(dirModeVal);
    }

    syncMenu();
}


void
GUI_PlayModeButton::onPlayModeMenuShown(QVariantMap md)
{
    // RLP_LOG_DEBUG(md)

    GUI_MenuPane* m = qobject_cast<GUI_MenuPane*>(menu());

    // sigh insane hack
    QPointF ppos = mapToScene(parentItem()->parentItem()->position());
    menu()->setPos(ppos.x() + 25, m->pos().y());
}