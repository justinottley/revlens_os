//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include <iostream>

#include "RlpExtrevlens/RLANN_DS/BrushBase.h"


RLP_SETUP_LOGGER(ext, RLANN_DS, BrushBase)

RLANN_DS_BrushBase::RLANN_DS_BrushBase(QString name):
    _name(name),
    _view(nullptr),
    _annCntrl(nullptr),
    _connected(false),
    _playstate(-1)
{
}


void
RLANN_DS_BrushBase::activate(DISP_GLView* view, RLANN_DS_DrawControllerBase* annCntrl)
{
    RLP_LOG_DEBUG("")

    if (_view != view)
    {
        _view = view;
        _annCntrl = annCntrl;
    }

    _view->setCursor(Qt::BlankCursor);

    if (!_connected)
    {
        connect(
            annCntrl->getMainController(),
            SIGNAL(playStateChanged(int)),
            this,
            SLOT(onPlayStateChanged(int))
        );

        _connected = true;
    }
}


void
RLANN_DS_BrushBase::setupFrame(RLANN_DS_AnnotationPtr ann)
{
    RLP_LOG_DEBUG("")
}


void
RLANN_DS_BrushBase::strokeEnd(RLANN_DS_AnnotationPtr ann)
{
    RLP_LOG_DEBUG("")
    if (ann != nullptr)
    {
        ann->saveToHistory();
    }
}


void
RLANN_DS_BrushBase::onPlayStateChanged(int state)
{
    RLP_LOG_DEBUG(state)

    _playstate = state;
}
