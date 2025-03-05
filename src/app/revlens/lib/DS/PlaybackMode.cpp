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
    _loopMode(LOOP)
{
}


qlonglong
DS_PlaybackMode::getNextFrameNum(qlonglong frame, DirectionMode direction)
{
    qlonglong newFrame = frame;
    
    if (direction == DS_PlaybackMode::FORWARD) {
        
        // std::cout << "FORWARD: " << _frameInc << " end: " << _endFrame << std::endl;

        newFrame += _frameInc;
        
        if (newFrame > _endFrame) {
            
            if (_loopMode == LOOP) {
                newFrame = _startFrame;
                
            } else if (_loopMode == NONE) {
                newFrame = INVALID_FRAME;
                
            }
        }
        
        
    } else if (direction == DS_PlaybackMode::BACKWARD) {
        
        newFrame -= _frameInc;
        
        if (newFrame < _startFrame) {
            
            if (_loopMode == LOOP) {
                newFrame = _endFrame;
                
            } else if (_loopMode == NONE) {
                newFrame = INVALID_FRAME;
                
            }
        }
    }
    
    return newFrame;
}


qlonglong
DS_PlaybackMode::clampFrame(qlonglong inputFrame)
{
    qlonglong result = inputFrame;
    
    if (result < _startFrame) {
        result = _startFrame;
        
    } else if (result > _endFrame) {
        result = _endFrame;
        
    }
    
    return result;
}


qlonglong
DS_PlaybackMode::getFrameCount()
{
    return (_endFrame - _startFrame) + 1;
}