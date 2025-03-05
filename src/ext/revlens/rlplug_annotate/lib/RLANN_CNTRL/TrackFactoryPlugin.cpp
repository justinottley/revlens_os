//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include "RlpExtrevlens/RLANN_CNTRL/TrackFactoryPlugin.h"

#include "RlpExtrevlens/RLANN_DS/PaintBrush.h"
#include "RlpExtrevlens/RLANN_DS/EraserBrush.h"
#include "RlpExtrevlens/RLANN_DS/TextBrush.h"
#include "RlpExtrevlens/RLANN_DS/PickerBrush.h"

#include "RlpExtrevlens/RLANN_MP/Brush.h"
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

    RLANN_DS_PaintBrush* paintBrush = new RLANN_DS_PaintBrush();
    _brushMap.insert(paintBrush->name(), paintBrush);

    RLANN_DS_EraserBrush* eraserBrush = new RLANN_DS_EraserBrush();
    _brushMap.insert(eraserBrush->name(), eraserBrush);

    RLANN_DS_TextBrush* textBrush = new RLANN_DS_TextBrush();
    _brushMap.insert(textBrush->name(), textBrush);

    RLANN_DS_PickerBrush* pickerBrush = new RLANN_DS_PickerBrush();
    _brushMap.insert(pickerBrush->name(), pickerBrush);

    RLANN_MP_Brush* mypaintBrush = new RLANN_MP_Brush();
    RLANN_MP_Handler* mph = RLANN_MP_Handler::instance();

    connect(
        mph,
        &RLANN_MP_Handler::brushRequested,
        mypaintBrush,
        &RLANN_MP_Brush::onBrushRequested
    );

    _brushMap.insert(mypaintBrush->name(), mypaintBrush);
}


RLANN_DS_BrushBase*
RLANN_CNTRL_TrackFactoryPlugin::getBrush(QString brushName)
{
    if (_brushMap.contains(brushName)) {
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
    track->setBrush(getBrush("BasicPaint"));
    track->brush()->updateSettings(_brushSettings);

    return DS_TrackPtr(track);
}