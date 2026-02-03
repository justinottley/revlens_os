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
    // RLP_LOG_DEBUG("")

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


bool
RLQTDISP_PanZoom::touchEventInternal(QTouchEvent* event)
{
    // RLP_LOG_DEBUG(event)

    bool result = true;

    if (event->type() == QEvent::TouchEnd)
    {
        _inZoom = false;
        _inPan = false;

        // RLP_LOG_DEBUG("TouchEnd")
        // RLP_LOG_DEBUG("SAVING" << _lastTouchZoomX)
        emit panZoomEnd();

        return result;
    }


    if (event->pointCount() != 2)
    {
        return false;
    }

    if (event->type() == QEvent::TouchBegin)
    {
        // nothing
    } else if (event->type() == QEvent::TouchUpdate)
    {
        QList<QEventPoint> points = event->points();

        QPointF pressPos1 = points.at(0).pressPosition();
        QPointF currPos1 = points.at(0).position();

        QPointF pressPos2 = points.at(1).pressPosition();
        QPointF currPos2 = points.at(1).position();

        
        QPointF pDiff1 = currPos1 - pressPos1;
        QPointF pDiff2 = currPos2 - pressPos2;

        QPointF pressDiff = pressPos2 - pressPos1;
        QPointF currDiff = currPos2 - currPos1;

        // qreal mh1 = pDiff1.manhattanLength();
        // qreal mh2 = pDiff2.manhattanLength();
        qreal mhPress = pressDiff.manhattanLength();
        qreal mhCurr = currDiff.manhattanLength();

        qreal diffX1 = pDiff1.x();
        qreal diffX2 = pDiff2.x();

        int diffX = diffX1;
        if (pressPos2.x() > pressPos1.x())
        // if (diffX2 > diffX1)
        {
            diffX = diffX2;
        }


        qreal mhdiff = std::abs(mhCurr - mhPress);

        // qreal mhNewDiff = std::abs(mhdiff - _lastMhDiff);

        // RLP_LOG_DEBUG("mhdiff:" << mhdiff << "mhnewDiff:" << mhNewDiff << "lastMhDiff:" << _lastMhDiff << "mhpress:" << mhPress << "pressdiff:" << pressDiff << "pos:" << pressPos2 << pressPos1)

        // if ((!_inZoom) && (mhPress > 18) && (mhdiff <= 15) && (_lastMhDiff > 0) && (_lastMhDiff <= 18) && (mhNewDiff <= 18))
        if ((!_inZoom) && (mhPress < 150) && (mhdiff <= 15))
        {
            _inPan = true;
            _inZoom = false;
        }
        else if (!_inPan) // && (mhdiff >= 18))
        {
            _inPan = false;
            _inZoom = true;
        }

        // _lastMhDiff = mhdiff;

        if (_inPan)
        {

            // RLP_LOG_DEBUG("PAN:" << currDiff)
            // pan
            _inPan = true;
            _inZoom = false;
            emit panChanged(currPos2 - pressPos2);
        }
        else if ((!_inPan) && (_inZoom))
        {
            // zoom
            _inPan = false;
            _inZoom = true;

            // _currTouchZoomX = zoomDiffX;
            // RLP_LOG_DEBUG("ZOOM:" << diffX)

            emit zoomChanged(diffX);
        }
        

        _display->update();


        // RLP_LOG_DEBUG(pressPos1 << currPos1 << pressPos2 << currPos2)

    }

    return result;
}