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
    _activatedDisplays.clear();
}


RLANN_DS_BrushBase*
RLANN_DS_BrushBase::clone()
{
    return new RLANN_DS_BrushBase(_name);
}


void
RLANN_DS_BrushBase::activate(DISP_GLView* view, RLANN_DS_DrawControllerBase* annCntrl)
{
    // RLP_LOG_DEBUG(annCntrl)

    if (_view != view)
    {
        _view = view;

        QString viewUuid = view->uuid();
        if (!_activatedDisplays.contains(viewUuid))
        {
            // RLP_LOG_DEBUG("Activating new display:" << viewUuid)

            connect(
                view,
                &DISP_GLView::grabImageReady,
                this,
                &RLANN_DS_BrushBase::setImage
            );

            connect(
                view,
                &DISP_GLView::displayDestroyed,
                this,
                &RLANN_DS_BrushBase::deactivate
            );

            _activatedDisplays.insert(viewUuid, true);
        }
    }

    if (_annCntrl != annCntrl)
    {
        _annCntrl = annCntrl;
    }

    // _view->setCursor(Qt::BlankCursor);

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
RLANN_DS_BrushBase::deactivate(DISP_GLView* view)
{
    disconnect(
        view,
        &DISP_GLView::grabImageReady,
        this,
        &RLANN_DS_BrushBase::setImage
    );

    disconnect(
        view,
        &DISP_GLView::displayDestroyed,
        this,
        &RLANN_DS_BrushBase::deactivate
    );

    QString viewUuid = view->uuid();
    _activatedDisplays.remove(viewUuid);

    if (_view == view)
    {
        RLP_LOG_DEBUG(_name << this << "Deactivating view" << view)

        _view = nullptr;
    }

    
}


void
RLANN_DS_BrushBase::setupFrame(RLANN_DS_AnnotationPtr ann)
{
    RLP_LOG_DEBUG("")
}


void
RLANN_DS_BrushBase::setImage(QImage img)
{
    // RLP_LOG_DEBUG("")
}


void
RLANN_DS_BrushBase::strokeMove(QPointF& point, QVariantMap mdata)
{
    if (_playstate == 0) // playing
    {
        return;
    }

    if (_view == nullptr)
    {
        // RLP_LOG_ERROR("NO VIEW")
        return;
    }

    _view->setCursor(Qt::ArrowCursor);
    _view->update();
}


void
RLANN_DS_BrushBase::strokeEnd(RLANN_DS_AnnotationPtr ann, QVariantMap mdata)
{
    // RLP_LOG_DEBUG("")
    if (ann != nullptr)
    {
        ann->saveToHistory();
    }
}


void
RLANN_DS_BrushBase::onPlayStateChanged(int state)
{
    // RLP_LOG_DEBUG(state)

    _playstate = state;
}
