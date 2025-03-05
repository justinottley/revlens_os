'''
Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
'''

import functools

import revlens
import revlens.cmds

from rlp.Qt import QStr, QVMap
from rlp import QtCore

TRACK_TYPE_ANNOTATE = 1

# For linter - exported by python binding
#
RLANN_CNTRL_DRAWCONTROLLER_OBJ = None

# ----

class AnnotateEventHandler:
    '''
    Intermediary to listen to events from central controllers to dispatch to python
    '''
    
    _INSTANCE = None

    def __init__(self):
        self.act_map = {
            'new_drawing': self.handle_new_drawing,
            'focus_on_frame': self.handle_focus_on_frame
        }


    @classmethod
    def instance(cls):
        if not cls._INSTANCE:
            cls._INSTANCE = AnnotateEventHandler()

        return cls._INSTANCE


    def init_connections(self):

        print('AnnotateEventHandler - init_connections!!! ')
        revlens.TL_CNTRL.timelineViewCreated.connect(
            self.on_timeline_view_created
        )


    def on_track_action_requested(self, view, info):

        if info['act_name'] in self.act_map:
            self.act_map[info['act_name']](view, info)



    def on_tl_track_added(self, view, info):

        if info['track_type'] == 'Annotation':

            tl_track = view.trackManager().getTrackByUuid(
                info['track_uuid'])

            tl_track.actionRequested.connect(
                functools.partial(
                    self.on_track_action_requested,
                    view
                )
            )

            pass


    def on_timeline_view_created(self, tl_uuid):

        view = revlens.TL_CNTRL.getTimelineView(tl_uuid)
        view.trackManager().trackAdded.connect(functools.partial(
            self.on_tl_track_added,
            view
        ))

    # -----

    def handle_new_drawing(self, view, info):
        print('Annotate: NEW DRAWING: {}'.format(info))

        track_uuid = info['track_uuid']
        curr_frame = revlens.cmds.get_current_frame_num()
        ann = RLANN_CNTRL_DRAWCONTROLLER_OBJ.initAnnotation(curr_frame, track_uuid, 960, 540)
        print(ann)
        ann.load('/home/justinottley/Documents/panel_base.png')


    def handle_focus_on_frame(self, view, info):
        print('Annotate: FOCUS ON ANNOTATION {} {}'.format(view, info))

        curr_frame = revlens.cmds.get_current_frame_num()

        start = curr_frame - 5
        if start < 1:
            start = 1
        
        sstart = str(start)
        send = '{}'.format(curr_frame + 5)

        view.buttonBar().visStartFrame().setText(sstart)
        view.buttonBar().visEndFrame().setText(send)
        view.buttonBar().setVisFrameRange({})
        view.buttonBar().visStartFrame().textArea().update()
        view.buttonBar().visEndFrame().textArea().update()
        view.setFocusedOnFrame(True)
        view.update()

        revlens.cmds.goto_frame(curr_frame)
