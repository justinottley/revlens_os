//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include "RlpExtrevlens/RLANN_CNTRL/TrackFactoryPlugin.h"

#include "RlpExtrevlens/RLANN_BRUSH/PaintBrush.h"
#include "RlpExtrevlens/RLANN_BRUSH/EraserBrush.h"
#include "RlpExtrevlens/RLANN_BRUSH/TextBrush.h"
#include "RlpExtrevlens/RLANN_BRUSH/PickerBrush.h"

#include "RlpExtrevlens/RLANN_MP/Brush.h"
#include "RlpExtrevlens/RLANN_MP/EraserBrush.h"
#include "RlpExtrevlens/RLANN_MP/Handler.h"

RLP_SETUP_EXT_LOGGER(RLANN_CNTRL, TrackFactoryPlugin)

RLANN_CNTRL_TrackFactoryPlugin::RLANN_CNTRL_TrackFactoryPlugin():
    DS_TrackFactoryPlugin("Annotation")
{
    initBrushes();
}


void
RLANN_CNTRL_TrackFactoryPlugin::initBrushes()
{
    RLP_LOG_DEBUG("");

    RLANN_DS_BrushBase* noBrush = new RLANN_DS_BrushBase("NONE");
    _brushMap.insert(noBrush->name(), noBrush);

    RlpAnnBrush_Paint* paintBrush = new RlpAnnBrush_Paint();
    _brushMap.insert(paintBrush->name(), paintBrush);

    RlpAnnBrush_Eraser* eraserBrush = new RlpAnnBrush_Eraser();
    _brushMap.insert(eraserBrush->name(), eraserBrush);

    RlpAnnBrush_Text* textBrush = new RlpAnnBrush_Text();
    _brushMap.insert(textBrush->name(), textBrush);

    RlpAnnBrush_Picker* pickerBrush = new RlpAnnBrush_Picker();
    _brushMap.insert(pickerBrush->name(), pickerBrush);

    RLANN_MP_Brush* mypaintBrush = new RLANN_MP_Brush();
    _brushMap.insert(mypaintBrush->name(), mypaintBrush);

    RLANN_MP_EraserBrush* mypaintEraserBrush = new RLANN_MP_EraserBrush();
    _brushMap.insert(mypaintEraserBrush->name(), mypaintEraserBrush);

    RLANN_MP_Handler* mph = RLANN_MP_Handler::instance();

}


RLANN_DS_BrushBase*
RLANN_CNTRL_TrackFactoryPlugin::getBrush(QString brushName)
{
    if (_brushMap.contains(brushName))
    {
        return _brushMap.value(brushName);
    }

    RLP_LOG_WARN(brushName << " - invalid brush");
    return nullptr;
}


bool
RLANN_CNTRL_TrackFactoryPlugin::hasBrush(QString name)
{
    return _brushMap.contains(name);
}


void
RLANN_CNTRL_TrackFactoryPlugin::setBrushSettings(QVariantMap settings)
{
    RLP_LOG_DEBUG(settings)

    _brushSettings = settings;

    BrushMapIterator it;
    for (it = _brushMap.begin(); it != _brushMap.end(); ++it)
    {
        it.value()->updateSettings(settings);
    }
}


DS_TrackPtr
RLANN_CNTRL_TrackFactoryPlugin::runCreateTrack(DS_Session* session, int trackIdx, QUuid uuid, QString owner)
{
    RLP_LOG_DEBUG(trackIdx);

    RLANN_DS_Track* track = new RLANN_DS_Track(session, trackIdx, uuid, owner);

    BrushMapIterator it;
    for (it = _brushMap.begin(); it != _brushMap.end(); ++it)
    {
        track->registerBrush(it.value()->clone());
    }

    track->setBrush("MyPaint");
    track->brush()->updateSettings(_brushSettings);

    return DS_TrackPtr(track);
}