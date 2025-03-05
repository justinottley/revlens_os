'''
Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
'''

import logging

import revlens
import revlens.plugin
import rlplug_qtbuiltin

from rlp import QtCore

rlogger = logging.getLogger('rlp.rlplug_qtbuiltin.fademessage')


def onTrackDataChanged(eventName, trackUuid, evtInfo):

    if (eventName == 'media_added'):
        msg = 'Loaded {}'.format(evtInfo['node_properties']['media.name'])

        rlplug_qtbuiltin.RLQTDISP_FADEMESSAGEPLUGIN_OBJ.displayMessage(
            msg, 1400, '')



def onNoticeEvent(noticeName, noticeInfo):

    if (noticeName == 'command_denied'):
        msg = 'Denied: {} - {}'.format(
            # noticeInfo['cmd.category'],
            noticeInfo['cmd.name'],
            noticeInfo['msg']
        )
        rlplug_qtbuiltin.RLQTDISP_FADEMESSAGEPLUGIN_OBJ.displayMessage(
            msg, 1400, 'red')
        
    if (noticeName == 'permissions_changed'):
        stateStr = 'ON'
        if not noticeInfo['value']:
            stateStr = 'OFF'

        msg = '{} - {} {}'.format(noticeInfo['username'], noticeInfo['category'], stateStr)
        rlplug_qtbuiltin.RLQTDISP_FADEMESSAGEPLUGIN_OBJ.displayMessage(
            msg, 1400, '')


def startup():

    fmPluginObj = rlplug_qtbuiltin.RLQTDISP_FADEMESSAGEPLUGIN_OBJ

    revlens.plugin.register(
        revlens.plugin.PLUGIN_EVENT,
        fmPluginObj
    )

    revlens.CNTRL.playStateChanged.connect(fmPluginObj.onPlayStateChanged)
    revlens.CNTRL.volumeChanged.connect(fmPluginObj.onVolumeChanged)
    revlens.CNTRL.noticeEvent.connect(onNoticeEvent)

    # revlens.CNTRL.sessionChanged.connect(on_session_changed)

    session = revlens.CNTRL.session()
    session.trackDataChanged.connect(onTrackDataChanged)

    rlogger.info('message connected')


