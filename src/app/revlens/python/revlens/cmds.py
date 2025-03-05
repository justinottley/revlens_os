'''
Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
'''
import os
import logging

from rlp.Qt import QStr
import rlp.util

import revlens
import revlens.media

LOG = logging.getLogger('rlp.{n}'.format(n=__name__))


def display_message(msg):
    if type(msg) == list:
        msg = msg[0]

    # NOTE: late import..
    import rlplug_qtbuiltin.cmds

    rlplug_qtbuiltin.cmds.display_message(msg)


def set_in_frame():
    '''
    Set global timeline in frame at current timeline mouse position
    '''

    timeline = revlens.TL_CNTRL.getFocusedTimelineView()

    mouse_frame = timeline.currMouseFrame()
    
    msg = 'Setting in point: {f}'.format(f=mouse_frame)
    LOG.verbose(msg)

    # controller update controlled by signal-slot connection
    revlens.CNTRL.setInFrame(mouse_frame, False, True)

    display_message(msg)



def set_out_frame():
    '''
    Set global timeline out frame at current timeline mouse position
    '''

    timeline = revlens.TL_CNTRL.getFocusedTimelineView()
    mouse_frame = timeline.currMouseFrame()
    
    msg = 'Setting out point: {f}'.format(f=mouse_frame)
    LOG.verbose(msg)

    # controller update controlled by signal-slot connection
    revlens.CNTRL.setOutFrame(mouse_frame, False, True)

    display_message(msg)


def clear_in_out_frames():
    
    msg = 'Clearing In/Out points'
    LOG.verbose(msg)
    
    curr_frame = revlens.cmds.get_current_frame_num()

    timeline = revlens.TL_CNTRL.getFocusedTimelineView()

    # controller update controller by signal-slot connection
    #
    revlens.CNTRL.setInFrame(1, False, True)
    revlens.CNTRL.setOutFrame(timeline.frameCount(), False, True)
    revlens.cmds.goto_frame(curr_frame)

    display_message(msg)


def toggle_play_pause():
    '''
    Toggle play state
    '''
    return revlens.CNTRL.runCommand('TogglePlayState', None)


def play():
    return revlens.CNTRL.play()

def pause():
    return revlens.CNTRL.pause()

def get_current_frame_num():
    '''
    Get the current frame number
    '''
    return revlens.CNTRL.getVideoManager().currentFrameNum()

def get_track_names(track_type=revlens.TRACK_TYPE_MEDIA):

    LOG.info('get_tracks_names()')

    session = revlens.CNTRL.session()
    result = []
    num_tracks = session.numTracks()
    for track_idx in range(num_tracks):
        track_obj = session.getTrackByIndex(track_idx)
        if track_obj.trackType() == track_type:
            result.append(track_obj.name())

    return result


def goto_frame(frameNum, recenter=True, schedule=True, emitAction=True):
    return revlens.CNTRL.gotoFrame(frameNum, recenter, schedule, emitAction)

def identify_media(media_in):
    return revlens.CNTRL.getMediaManager().identifyMedia(
        revlens.media.locate(media_in))


def load_media_list(media_list, track_idx=0, start_frame=1, clear=False, *args, **kwargs):
    '''
    Main entry point for loading media into revlens.
    '''

    if not hasattr(media_list, '__iter__'):
        media_list = [media_list]
        
    return revlens.media.load_media_list(
        media_list,
        track_idx=track_idx,
        start_frame=start_frame,
        clear=clear,
        *args, **kwargs)


def insert_media_at_frame(media_in, track_idx, frame_num, identify=False):
    LOG.info('insert_media_at_frame(): track: {t} frame: {f}'.format(t=track_idx, f=frame_num))
    return revlens.media.insert_media_at_frame(media_in, track_idx, frame_num, identify=identify)


def export(start_frame,
           end_frame,
           output_path,
           output_width,
           output_height,
           frame_format='png',
           frame_prefix='export',
           output_format='mov',
           create_edb_media=False,
           media_title='Export Media',
           comment=''):

    app_globals = rlp.util.APPGLOBALS.globals()
    app_rootpath = app_globals['app.rootpath']

    import tempfile
    temp_dir = tempfile.mkdtemp(prefix = 'revlens_export_')
    session_path = os.path.join(temp_dir, 'session.rls')

    revlens.CNTRL.getMediaManager().saveSessionFile(session_path, False)
    print('Saved {}'.format(session_path))

    batch_template_path = os.path.join(app_rootpath, 'revlens', 'etc', 'batch_template.py')
    batch_template = open(batch_template_path).read()

    batch_template = batch_template.replace('__SCENE__', session_path)
    batch_template = batch_template.replace('__START_FRAME__', str(start_frame))
    batch_template = batch_template.replace('__END_FRAME__', str(end_frame))
    batch_template = batch_template.replace('__OUTPUT_PATH__', output_path)
    batch_template = batch_template.replace('__OUTPUT_WIDTH__', str(output_width))
    batch_template = batch_template.replace('__OUTPUT_HEIGHT__', str(output_height))
    batch_template = batch_template.replace('__OUTPUT_FORMAT__', str(output_format))

    batch_template = batch_template.replace('__FRAME_PREFIX__', str(frame_prefix))
    batch_template = batch_template.replace('__FRAME_FORMAT__', str(frame_format))
    
    batch_template = batch_template.replace('__CREATE_EDB_MEDIA__', str(create_edb_media))
    batch_template = batch_template.replace('__MEDIA_TITLE__', media_title)
    batch_template = batch_template.replace('__COMMENT__', comment)

    batch_script_path = os.path.join(temp_dir, 'export.py')
    with open(batch_script_path, 'w') as wfh:
        wfh.write(batch_template)
        print('Wrote {}'.format(batch_script_path))

    revlens_cmd = 'revlens --batch --script {}'.format(batch_script_path)
    print(revlens_cmd)
    os.system(revlens_cmd)


def exit_presentation_mode():
    if revlens.GUI.hasPresentationViewer():
        revlens.GUI.presentationWindow().close()

def force_quit():
    revlens.CNTRL.forceQuit()