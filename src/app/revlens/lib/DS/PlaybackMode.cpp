//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include <iostream>

#include "RlpRevlens/DS/PlaybackMode.h"

RLP_SETUP_LOGGER(revlens, DS, PlaybackMode)

DS_PlaybackMode::DS_PlaybackMode() :
    _startFrame(1L),
    _endFrame(1L),
    _frameInc(1L),
    _directionMode(FORWARD),
    _loopMode(LOOP),
    _session(nullptr)
{
}


void
DS_PlaybackMode::setLoopMode(LoopMode loopMode)
{
    // RLP_LOG_DEBUG(int(loopMode))

    _loopMode = loopMode;
}


void
DS_PlaybackMode::setFrameRange(qlonglong startFrame, qlonglong endFrame)
{
    // RLP_LOG_DEBUG(startFrame << " " << endFrame);

    _startFrame = startFrame;
    _endFrame = endFrame;
}


void
DS_PlaybackMode::setInFrame(qlonglong inFrame)
{
   // RLP_LOG_DEBUG(inFrame);

    _startFrame = inFrame;
}


void
DS_PlaybackMode::setOutFrame(qlonglong outFrame)
{
    // RLP_LOG_DEBUG(outFrame);

    _endFrame = outFrame;
}


std::pair<qlonglong, DS_PlaybackMode::DirectionMode>
DS_PlaybackMode::getNextFrameNum(qlonglong frame, DirectionMode directionMode, IncMode incMode)
{
    // RLP_LOG_DEBUG(frame << _endFrame << (int)incMode)

    qlonglong newFrame = frame;
    DS_PlaybackMode::DirectionMode nextDirection = directionMode;

    if (incMode == INC)
    {
        // std::cout << "FORWARD: " << _frameInc << " end: " << _endFrame << std::endl;

        if (directionMode == FORWARD)
        {
            newFrame += _frameInc;

            if (newFrame > _endFrame)
            {

                if (_loopMode == LOOP)
                {
                    newFrame = _startFrame;

                }
                else if (_loopMode == NONE)
                {
                    newFrame = INVALID_FRAME;

                }
                else if (_loopMode == SWING)
                {

                    // RLP_LOG_DEBUG("reversing direction from forward to backward")
                    nextDirection = BACKWARD;

                    newFrame = _endFrame - _frameInc;
                }
            }
        }
        else if (directionMode == BACKWARD)
        {
            newFrame -= _frameInc;

            if (newFrame < _startFrame)
            {
                if (_loopMode == LOOP)
                {
                    newFrame = _endFrame;
                }
                else if (_loopMode == NONE)
                {
                    newFrame = INVALID_FRAME;
                }
                else if (_loopMode == SWING)
                {
                    // RLP_LOG_DEBUG("reversing direction from backward to forward")
                    nextDirection = FORWARD;

                    newFrame = _startFrame + _frameInc;
                }
            }
        }
    }

    else if (incMode == DEC)
    {
        if (directionMode == FORWARD)
        {
            newFrame -= _frameInc;
        }
        else if (directionMode == BACKWARD)
        {
            newFrame += _frameInc;
        }

        if (newFrame < _startFrame)
        {

            if (_loopMode == LOOP)
            {
                newFrame = _endFrame;

            }
            else if (_loopMode == NONE)
            {
                newFrame = INVALID_FRAME;

            }
        }

        else if (newFrame > _endFrame)
        {
            if (_loopMode == LOOP)
            {
                newFrame = _startFrame;
            }
            else if (_loopMode == NONE)
            {
                newFrame = INVALID_FRAME;
            }
        }
    }

    std::pair<qlonglong, DirectionMode> result = {newFrame, nextDirection};
    return result;
}


qlonglong
DS_PlaybackMode::clampFrame(qlonglong inputFrame)
{
    qlonglong result = inputFrame;

    if (result < _startFrame)
    {
        result = _startFrame;

    } else if (result > _endFrame)
    {
        result = _endFrame;

    }

    return result;
}


qlonglong
DS_PlaybackMode::getFrameCount()
{
    return (_endFrame - _startFrame) + 1;
}