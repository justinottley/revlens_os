'''
Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
'''
import pprint
import logging

from rlp import QtCore, QtGui

import revlens
import revlens.cmds

import rlplug_qtbuiltin
import rlplug_qtbuiltin.hud


rlogger = logging.getLogger('rlp.rlplug_qtbuiltin.cmds')

def gotoPreviousFrame():
    revlens.CNTRL.gotoFrame(
        revlens.cmds.get_current_frame_num() - 1, True, False, True)


    # revlens.cmds.goto_frame(
    #     revlens.cmds.get_current_frame_num() - 1)
    # revlens.cmds.goto_frame(
    #     revlens.CNTRL.getVideoManager().playbackMode().clampFrame(
    #         revlens.cmds.get_current_frame_num() + 1))


def gotoNextFrame():
    revlens.CNTRL.gotoFrame(
        revlens.cmds.get_current_frame_num() + 1, True, False, True)

    # revlens.cmds.goto_frame(
    #     revlens.CNTRL.getVideoManager().playbackMode().clampFrame(
    #         revlens.cmds.get_current_frame_num() - 1))

def toggleFpsHud():

    print('Error: toggleFpsHud() broken, requires display focus widget') # rlogger.info('toggleFpsHud()')
    return

    
    # curr_visible = rlplug_qtbuiltin.RLQTDISP_HUD_OBJ.isHudVisible("min")
    # rlplug_qtbuiltin.RLQTDISP_HUD_OBJ.setHudVisible("min", not curr_visible)
    


def toggleMediaHud():

    return rlplug_qtbuiltin.hud.toggleMediaHud()

def toggleHud():

    # print('Error: toggleHud() broken, requires display focus widget') # rlogger.info('toggleFpsHud()')
    # return

    # curr_visible = rlplug_qtbuiltin.RLQTDISP_HUD_OBJ.isHudVisible("sginfo")
    # rlplug_qtbuiltin.RLQTDISP_HUD_OBJ.setHudVisible('sginfo', not curr_visible)

    payload = {'action': 'toggle_vis'}
    revlens.CNTRL.emitNoticeEvent('view.request_hud', payload)


def display_message(message, duration=1400, colour=''):

    if type(message) == list:
        message = message[0]

    rlplug_qtbuiltin.RLQTDISP_FADEMESSAGEPLUGIN_OBJ.displayMessage(
        message, duration, colour)


def resetPanZoom():
    
    rlogger.verbose('Reset Pan/Zoom')
    
    for dx in range(revlens.CNTRL.getVideoManager().getDisplayCount()):
        paint_engine = revlens.CNTRL.getVideoManager().getDisplayByIdx(dx).getCurrentPaintEngine()
        paint_engine.resetPan()
        paint_engine.resetZoom()
        
    revlens.CNTRL.getVideoManager().update()
    
    display_message('Reset Pan/Zoom')
