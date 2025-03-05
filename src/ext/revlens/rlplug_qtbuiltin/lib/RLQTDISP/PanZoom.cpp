//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include "RlpExtrevlens/RLQTDISP/PanZoom.h"

RLP_SETUP_EXT_LOGGER(RLQTDISP, PanZoom)

RLQTDISP_PanZoom::RLQTDISP_PanZoom():
    DS_EventPlugin(QString("PanZoom"), true),
    _panStartPos(),
    _inPan(false),
    _inZoom(false)
{
}


DS_EventPlugin*
RLQTDISP_PanZoom::duplicate()
{
    // RLP_LOG_DEBUG("")

    RLQTDISP_PanZoom* childPlug = new RLQTDISP_PanZoom();
    childPlug->setMainController(_controller);

    _childPlugins.append(childPlug);

    return childPlug;
}


bool
RLQTDISP_PanZoom::setDisplay(DISP_GLView* display)
{
    DS_EventPlugin::setDisplay(display);

    connect(
        this,
        &RLQTDISP_PanZoom::panChanged,
        display,
        &DISP_GLView::panPosChanged);

    connect(
        this,
        &RLQTDISP_PanZoom::zoomChanged,
        display,
        &DISP_GLView::zoomPosChanged);

    connect(
        this,
        &RLQTDISP_PanZoom::panZoomEnd,
        display,
        &DISP_GLView::onPanZoomEnd);

    return true;
}


bool
RLQTDISP_PanZoom::inPan(QMouseEvent* event)
{
    return (event->modifiers().testFlag(Qt::ControlModifier) &&
            !event->modifiers().testFlag(Qt::ShiftModifier));
}


bool
RLQTDISP_PanZoom::inZoom(QMouseEvent* event)
{
    return (event->modifiers().testFlag(Qt::ControlModifier) &&
            event->modifiers().testFlag(Qt::ShiftModifier));
}


bool
RLQTDISP_PanZoom::mousePressEventInternal(QMouseEvent* event)
{
    RLP_LOG_DEBUG("")

    bool result = false;

    if (inPan(event))
    {

        _panStartPos = event->pos();
        _inPan = true;

        result = true;
    }
    else if (inZoom(event))
    {

        _zoomStartX = event->pos().x();
        _inZoom = true;
        result = true;
    }

    return result;
}


bool
RLQTDISP_PanZoom::mouseMoveEventInternal(QMouseEvent* event)
{
    bool result = false;

    if (_inPan)
    {

        QPointF panDiffPos = event->pos() - _panStartPos;

        // _paintEngine->setPanPos(panDiffPos);
        emit panChanged(panDiffPos);

        _display->update();

        result = true;
    }
    else if (_inZoom)
    {

        int zoomDiffX = event->pos().x() - _zoomStartX;
        // _paintEngine->setZoomPos(zoomDiffX);

        emit zoomChanged(zoomDiffX);

        _display->update();

        result = true;
    }

    return result;
}


bool
RLQTDISP_PanZoom::mouseReleaseEventInternal(QMouseEvent* event)
{
    bool result = false;

    if (_inPan || _inZoom)
    {

        _inPan = false;

        emit panZoomEnd();
        // emit
        // _paintEngine->setPanEnd();

        // _paintEngine->setZoomEnd();
        _inZoom = false;
    }
    else if (_inZoom)
    {
    }

    return false;
}
