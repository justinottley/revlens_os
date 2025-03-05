#include "RlpExtrevlens/RLANN_GUI/Track.h"

#include "RlpCore/PY/Interp.h"

#include "RlpGui/PANE/SvgButton.h"


#include "RlpExtrevlens/RLANN_GUI/AnnotationItem.h"
#include "RlpExtrevlens/RLANN_GUI/TrackActivateButton.h"

#include "RlpCore/UTIL/AppGlobals.h"
#include "RlpRevlens/GUI_TL2/TrackToggleButtonImage.h"


RLP_SETUP_EXT_LOGGER(RLANN_GUI, Track)

RLANN_GUI_Track::RLANN_GUI_Track(GUI_TL2_TrackManager* trackManager, DS_TrackPtr strack, QString trackName, RLANN_CNTRL_DrawController* drawController):
    GUI_TL2_Track(trackManager, strack, trackName),
    _drawController(drawController)
{

    initTrackConnections();
    initGearMenu();
    initButtons();
    
    #ifndef SCAFFOLD_WASM
    setButtonsVisible(false, /*optionalOnly=*/true);
    #endif

    setHeight(trackHeight());
    onParentSizeChanged(view()->width(), view()->height());
}


RLANN_GUI_Track::~RLANN_GUI_Track()
{
}


void
RLANN_GUI_Track::initTrackConnections()
{
    RLP_LOG_DEBUG("");
    
    connect(
        _strack.get(),
        SIGNAL(trackCleared()),
        this,
        SLOT(clearItems())
    );
}


qreal
RLANN_GUI_Track::trackHeight()
{
    qreal height = _heightDefault + 20;

    // RLP_LOG_DEBUG(height)

    return height;
    
}


void
RLANN_GUI_Track::initCurrSession()
{
    RLP_LOG_DEBUG("")

    RLANN_DS_Track* atrack = qobject_cast<RLANN_DS_Track*>(_strack.get());

    QList<QUuid> annUuids = atrack->getAnnotationUuids();

    QList<QUuid>::iterator it;
    for (it= annUuids.begin(); it != annUuids.end(); ++it)
    {
        QString annUuidStr = it->toString();
        RLP_LOG_DEBUG(annUuidStr)

        RLANN_DS_AnnotationPtr ann = atrack->getAnnotationByUuid(annUuidStr);

        if (!ann->isValid())
        {
            continue;
        }

        qlonglong startFrame = ann->startFrame();
        qlonglong endFrame = ann->endFrame();

        RLP_LOG_DEBUG(annUuidStr << startFrame << endFrame)

        QVariantList frameList;
        for (qlonglong sf=startFrame; sf <= endFrame; ++sf)
        {
            frameList.append(sf);
        }

        QVariantMap evtInfo;
        evtInfo.insert("frame_list", frameList);

        atrack->session()->emitTrackDataChanged("add_annotation", atrack->uuid().toString(), evtInfo);
        
    }

}


void
RLANN_GUI_Track::initGearMenu()
{
    RLP_LOG_DEBUG("");

    GUI_MenuItem* ghostingMenu = _gearMenu->addItem("Ghosting");
    ghostingMenu->menu()->setExclusiveSelect(true);

    GUI_MenuItem* zeroFrame = ghostingMenu->addAction("0");
    zeroFrame->insertDataValue("count", 0);
    zeroFrame->setCheckable(true);
    zeroFrame->setChecked(true);
    connect(
        zeroFrame,
        &GUI_MenuItem::menuItemSelected,
        this,
        &RLANN_GUI_Track::onGhostingChanged
    );

    GUI_MenuItem* oneFrame = ghostingMenu->addAction("1");
    oneFrame->insertDataValue("count", 1);
    oneFrame->setCheckable(true);
    connect(
        oneFrame,
        &GUI_MenuItem::menuItemSelected,
        this,
        &RLANN_GUI_Track::onGhostingChanged
    );

    GUI_MenuItem* twoFrame = ghostingMenu->addAction("2");
    twoFrame->insertDataValue("count", 2);
    twoFrame->setCheckable(true);
    connect(
        twoFrame,
        &GUI_MenuItem::menuItemSelected,
        this,
        &RLANN_GUI_Track::onGhostingChanged
    );

    GUI_MenuItem* newDrawing = _gearMenu->addItem("New Drawing");
    connect(
        newDrawing,
        SIGNAL(triggered()),
        this,
        SLOT(onNewDrawingRequested())
    );


    GUI_MenuItem* exportAnnotations = _gearMenu->addItem("Export Annotations..");
    connect(
        exportAnnotations,
        SIGNAL(triggered()),
        this,
        SLOT(onExportAnnotationsRequested())
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
RLANN_GUI_Track::initButtons()
{

    GUI_SvgToggleButton* visButton = new GUI_SvgToggleButton(
        ":feather/lightgrey/eye.svg",
        ":feather/lightgrey/eye-off.svg",
        this, 18, 0, GUI_Style::TrackHeaderFg
    );
    visButton->setToolTipText("Toggle Visibility");

    connect(
        visButton,
        &GUI_SvgButton::buttonPressed,
        this,
        &RLANN_GUI_Track::onVisibilityChanged
    );

    addButton("visibility", visButton);

    GUI_SvgButton* gearButton = new GUI_SvgButton(
        ":feather/lightgrey/settings.svg", this, 18, 0, GUI_Style::TrackHeaderFg
    );
    gearButton->setToolTipText("Gear Menu - click for available actions");
    
    connect(
        gearButton,
        &GUI_SvgButton::buttonPressed,
        this,
        &RLANN_GUI_Track::onGearMenuClicked
    );

    addButton("gear_menu", gearButton);
    GUI_TL2_TrackToggleButton* activateButton = new RLANN_GUI_TrackActivateButton(this);
    activateButton->setToolTipText("Active Annotation Track");

    connect(
        activateButton,
        SIGNAL(toggleChanged(bool)),
        this,
        SLOT(onActiveTrackToggled(bool))
    );

    addButton("activate", activateButton);

    /*
    if (strack->uuid() == _drawController->activeTrack()->uuid()) {
        activateButton->setToggled(true);
    }
    */

    connect(
        _drawController,
        SIGNAL(activeTrackChanged(QString)),
        activateButton,
        SLOT(onActiveTrackChanged(QString))
    );

    /*
    GUI_TL2_TrackToggleButtonImage* prevItemButton = new GUI_TL2_TrackToggleButtonImage(
        this,
        ":/PioFiveMusicWhite/saltar-a-pista-anterior_trackgray.png"
    );
    connect(
        prevItemButton,
        SIGNAL(toggleChanged(bool)),
        this,
        SLOT(onPrevItemClicked(bool))
    );
    */

    GUI_SvgButton* prevItemButton = new GUI_SvgButton(
        ":left-up.svg", this, 18, 0, GUI_Style::TrackHeaderFg
    );

    prevItemButton->setToolTipText("Go To Previous Annotation");
    prevItemButton->setMetadata("name", "prev_item");
    connect(
        prevItemButton, &GUI_SvgButton::buttonPressed,
        this, &RLANN_GUI_Track::onPrevItemClicked
    );

    addButton("prev_item", prevItemButton);

    /*
    GUI_TL2_TrackToggleButtonImage* nextItemButton = new GUI_TL2_TrackToggleButtonImage(
        this,
        ":/PioFiveMusicWhite/skip-track-option_trackgray.png"
    );

    connect(
        nextItemButton,
        SIGNAL(toggleChanged(bool)),
        this,
        SLOT(onNextItemClicked(bool))
    );
    */

    GUI_SvgButton* nextItemButton = new GUI_SvgButton(
        ":right-up.svg", this, 18, 0, GUI_Style::TrackHeaderFg
    );
    nextItemButton->setToolTipText("Go To Next Annotation");
    nextItemButton->setMetadata("name", "next_item");
    connect(
        nextItemButton, &GUI_SvgButton::buttonPressed,
        this, &RLANN_GUI_Track::onNextItemClicked
    );

    addButton("next_item", nextItemButton);

    

    GUI_SvgButton* focusButton = new GUI_SvgButton(":focus.svg", this, 18);
    focusButton->setToolTipText("Focus Timeline on Current Frame");
    focusButton->setMetadata("name", "focus_on_frame");
    addButton("focus_on_frame", focusButton);


}



RLANN_DS_Track*
RLANN_GUI_Track::strack()
{
    return dynamic_cast<RLANN_DS_Track*>(_strack.get());
}


QString
RLANN_GUI_Track::label()
{
    QString label = name();
    if (isRemote()) {
        label += " - ";
        label += _strack->owner();
    }

    return label;
}


bool
RLANN_GUI_Track::hasAnnotationItem(qlonglong frame)
{
    bool hasAnn = _itemFrameMap.contains(frame);
    // RLP_LOG_DEBUG(frame << " " << hasAnn);

    return hasAnn;
}


void
RLANN_GUI_Track::holdFrame(qlonglong srcFrame, qlonglong destFrame)
{
    if (_itemFrameMap.contains(srcFrame)) {
        QUuid annUuid = _itemFrameMap.value(srcFrame);
        _itemFrameMap.insert(destFrame, annUuid);

    } else {
        RLP_LOG_ERROR("no annotation found on source frame " << srcFrame);
    }
    
}


void
RLANN_GUI_Track::releaseFrame(qlonglong destFrame)
{
    if (_itemFrameMap.contains(destFrame)) {
        _itemFrameMap.remove(destFrame);

    } else {
        RLP_LOG_ERROR("No annotation found on frame " << destFrame);
    }
}


void
RLANN_GUI_Track::addAnnotationItem(RLANN_DS_AnnotationPtr ann, qlonglong frame)
{
    RLANN_GUI_AnnotationItem* item = new RLANN_GUI_AnnotationItem(this, ann, frame);
    
    addItem(item);
}


bool
RLANN_GUI_Track::clearAnnotation(QString annUuidStr)
{
    RLP_LOG_DEBUG(annUuidStr);

    RLANN_DS_AnnotationPtr ann = strack()->getAnnotationByUuid(annUuidStr);
    
    if (ann != nullptr) {

        RLP_LOG_INFO("Clearing annotation " << annUuidStr);
        QVariantList frameList = strack()->getFrameList(QUuid(annUuidStr));
        QVariantList::iterator it;
        for (it = frameList.begin(); it != frameList.end(); ++it) {

            RLP_LOG_DEBUG("clearing frame " << (*it).toLongLong());
            _itemFrameMap.remove((*it).toLongLong());
        }
    }

    return clearItem(annUuidStr);
}


void
RLANN_GUI_Track::onVisibilityChanged()
{
    GUI_ButtonBase* visButton = getButtonByName("visibility");
    bool vis = !visButton->isToggled();

    // RLP_LOG_DEBUG(vis);

    visButton->setToggled(vis);

    strack()->setEnabled(!vis);
    _drawController->updateDisplay();
}


void
RLANN_GUI_Track::onActiveTrackToggled(bool isActive)
{
    RLP_LOG_DEBUG(isActive);

    if (isActive) {
        _drawController->setActiveTrack(uuid().toString());
    }
}


void
RLANN_GUI_Track::onPrevItemClicked()
{
    // RLP_LOG_DEBUG("");

    CNTRL_MainController* cntrl = dynamic_cast<CNTRL_MainController*>(CNTRL_MainController::instance());

    qlonglong currFrame = cntrl->currentFrameNum();
    
    QString currAnnUuid;
    if (strack()->hasAnnotation(currFrame)) {
        currAnnUuid = strack()->getAnnotation(currFrame)->uuid().toString();
    }

    // RLP_LOG_DEBUG("curr annotation uuid: " << currAnnUuid);

    QList<qlonglong> frameList = strack()->getAnnotatedFrames();

    // Edge case
    if (currFrame > frameList.at(frameList.size() - 1)) {
        RLP_LOG_DEBUG("past last annotation, returning end annotation");

        cntrl->gotoFrame(frameList.at(frameList.size() - 1), true, true);
        return;
    }


    QList<qlonglong>::iterator it;
    qlonglong fidx = 0;
    bool found = false;
    for (it=frameList.begin(); it != frameList.end(); ++it) {
        if ((*it >= currFrame) && (fidx > 0)) {

            qlonglong sidx = fidx;

            // rewind until we get an annotation uuid different from the current
            //
            while (sidx > 0) {
                
                qlonglong prevAnnotatedFrame = frameList.at(sidx - 1);
                QString prevAnnUuid = strack()->getAnnotation(prevAnnotatedFrame)->uuid().toString();

                // RLP_LOG_DEBUG(sidx << " " << prevAnnotatedFrame << " " << prevAnnUuid << " " << currAnnUuid);

                if (prevAnnUuid != currAnnUuid) {
                    cntrl->gotoFrame(prevAnnotatedFrame, true, true);
                    found = true;
                    break;
                }
                
                sidx--;
            }

        }
        
        if (found) {
            break;
        }

        fidx++;
    }

    //if (!found) {
    //    qlonglong prevAnnotatedFrame = frameList.at(frameList.size() - 1);
    //}
}


void
RLANN_GUI_Track::onNextItemClicked()
{
    // RLP_LOG_DEBUG("");

    CNTRL_MainController* cntrl = dynamic_cast<CNTRL_MainController*>(CNTRL_MainController::instance());

    qlonglong currFrame = cntrl->currentFrameNum();
    
    QString currAnnUuid;
    if (strack()->hasAnnotation(currFrame)) {
        currAnnUuid = strack()->getAnnotation(currFrame)->uuid().toString();
    }

    QList<qlonglong> frameList = strack()->getAnnotatedFrames();

    QList<qlonglong>::iterator it;
    bool found = false;

    qlonglong fidx = 0;
    for (it=frameList.begin(); it != frameList.end(); ++it) {
        if (*it > currFrame) {

            qlonglong sidx = fidx;

            // advance until we get an annotation uuid different from the current
            //
            while (sidx < frameList.size()) {
                qlonglong nextAnnotatedFrame = frameList.at(sidx);
                QString nextAnnUuid = strack()->getAnnotation(nextAnnotatedFrame)->uuid().toString();

                // RLP_LOG_DEBUG(sidx << " " << nextAnnotatedFrame << " " << nextAnnUuid << " " << currAnnUuid);

                if (nextAnnUuid != currAnnUuid) {
                    cntrl->gotoFrame(nextAnnotatedFrame, true, true);
                    found = true;
                    break;
                }
                
                sidx++;

            }
        }

        if (found) {
            break;
        }

        fidx++;
    }
}


void
RLANN_GUI_Track::onExportAnnotationsRequested()
{
    RLP_LOG_DEBUG("");

    QVariantList args;
    QVariant tv;
    tv.setValue(_strack.get());
    args.append(tv);

    PY_Interp::call("rlplug_annotate.cmds.request_export_annotations", args);

    /*
    QInputDialog* dialog = new QInputDialog();

    dialog->setWindowTitle("revlens - Export Annotations");
    dialog->setLabelText("Enter directory to export to:");
    
    int result = dialog->exec();

    if (result) {
        QString exportDir = dialog->textValue();

        RLP_LOG_DEBUG("export to " <<  exportDir);

        QList<qlonglong> annFrames = strack()->getAnnotatedFrames();

        QList<qlonglong>::iterator it;
        QString lastAnnUuid;
        for (it=annFrames.begin(); it!=annFrames.end(); ++it) {

            RLANN_DS_AnnotationPtr ann = strack()->getAnnotation(*it);
            QString nextAnnUuid = ann->uuid().toString();

            RLP_LOG_DEBUG(nextAnnUuid);

            if (nextAnnUuid != lastAnnUuid) {

                RLP_LOG_DEBUG("Found annotation: " << nextAnnUuid);

                QString filePath = exportDir + "/";
                filePath += name();
                filePath += ".";
                filePath += QString("%1").arg((qlonglong)*it, 4, 10, QChar('0'));
                filePath += ".png";

                RLP_LOG_DEBUG(filePath);

                ann->save(filePath);

                lastAnnUuid = nextAnnUuid;
            }

        }
    }
    */

}


void
RLANN_GUI_Track::onNewDrawingRequested()
{
    RLP_LOG_DEBUG("")

    CNTRL_MainController* cntrl = dynamic_cast<CNTRL_MainController*>(CNTRL_MainController::instance());

    qlonglong currFrame = cntrl->currentFrameNum();
    RLANN_DS_Annotation* ann = _drawController->initAnnotation(currFrame, strack()->uuid().toString(), 960, 540);
    ann->load(":panel_base.png");
    ann->setValid();
}


void
RLANN_GUI_Track::onGhostingChanged(QVariantMap md)
{
    RLP_LOG_DEBUG(md)
    int ghostCount = md.value("data").toMap().value("count").toInt();

    RLP_LOG_DEBUG("Setting ghost frame count to:" << ghostCount)

    strack()->setGhostFrameCount(ghostCount);
}