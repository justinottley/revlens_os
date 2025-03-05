'''
Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
'''

import logging

from rlp.Qt import QStr
from rlp import QtCore
from rlp.gui.dialogs import InputDialog

import revlens
import revlens.cmds

import rlplug_annotate

rlogger = logging.getLogger('rlp.{ns}'.format(ns=__name__))

def createTrack(*args):
    revlens.CNTRL.session().addTrack('', 'Annotation')


def clearAnnotationOnCurrentFrame():
    curr_frame = revlens.cmds.get_current_frame_num()
    clearAnnotations(curr_frame)

    # TO get the drawcontroller to run setupFrame() so we can draw immediately again on the current frame
    revlens.cmds.goto_frame(curr_frame)


def clearAnnotations(frame, emitSyncAction=True, trackUuidStr=None):

    import rlplug_qtbuiltin.cmds
    
    rlogger.verbose('clearAnnotations: {} track uuid: {}'.format(
        frame, trackUuidStr))
    
    draw_cntrl = rlplug_annotate.RLANN_CNTRL_DRAWCONTROLLER_OBJ

    if trackUuidStr:
        track = revlens.CNTRL.session().getTrackByUuid(trackUuidStr).get()

    else:
        track = draw_cntrl.activeTrack()

    rlogger.verbose('track: {}'.format(track))

    if not track:
        rlogger.error('invalid track!')
        return


    track_uuid = track.uuid().toString()
    if (not track.hasAnnotation(frame)):
        rlogger.error('no annotation on frame {}'.format(frame))
        return

    ann = track.getAnnotationPtr(frame)
    ann_uuid = ann.uuid().toString()

    frame_list = track.getFrameListAsList(ann.uuid())
    tl_names = revlens.TL_CNTRL.viewNames()


    for tl_view in tl_names:
        tl = revlens.TL_CNTRL.getTimelineView(tl_view)
        gtrack = tl.trackManager().getTrackByUuid(track_uuid)
        gtrack.clearAnnotation(ann_uuid)

    track.clearAnnotationOnFrame(frame)
    
    revlens.CNTRL.getVideoManager().update()

    if emitSyncAction:
        
        revlens.CNTRL.emitSyncAction(
            'clearAnnotations',
            {
                'update_info': {
                    'frame': frame,
                    'track_uuid': track_uuid,
                    'ann_uuid': ann_uuid
                }
            }
        )

    rlogger.info('running display message')

    rlplug_qtbuiltin.cmds.display_message('Cleared Annotation: {n} frames'.format( n=len(frame_list)))


def export_annotations_to(track, export_dir):

    print('export annotations to: {}'.format(export_dir))

    ann_frames = track.getAnnotatedFramesAsList()
    print(ann_frames)

    last_ann_uuid = None
    for ann_frame in ann_frames:
        ann = track.getAnnotationPtr(ann_frame)
        ann_uuid = ann.uuid().toString()
        if ann_uuid != last_ann_uuid:

            print('Found annotation: {}'.format(ann_uuid))
            ann_path = '{}/{}.{}.png'.format(
                export_dir,
                track.name(),
                str(ann_frame).zfill(3)
            )
            print(ann_path)

            ann.save(ann_path)


def request_export_annotations(track):

    dialog = InputDialog.create(
        'Export Annotations',
        'Output Dir:',
        accept_label='Export'
    )

    def _on_accept(md):
        export_dir = md['value']
        export_annotations_to(track, export_dir)


    dialog.accept.connect(_on_accept)
