
import os
import tempfile
import traceback

from rlp.Qt import QStr
from rlp import QtCore
from rlp.gui.dialogs import ConfirmDialog, InputDialog


import revlens
import revlens.gui
import revlens.cmds
import revlens.media
from revlens.keyboard import KeyBinding

# C++ enums
TL_MODE_SCRUB = 0
TL_MODE_EDIT = 1

def on_cache_visibility_toggled():
    print('on_cache_visibility_toggled')
    tl_view = revlens.TL_CNTRL.getFocusedTimelineView()
    tl_view.setCacheVisible(not tl_view.isCacheVisible())
    tl_view.updateInternal()


def on_tick_lines_visibility_toggled():
    print('on_tick_lines_toggled')
    tl_view = revlens.TL_CNTRL.getFocusedTimelineView()
    tl_view.setTickLinesVisible(not tl_view.isTickLinesVisible())
    tl_view.updateInternal()



def on_tick_numbers_visibility_toggled():
    tl_view = revlens.TL_CNTRL.getFocusedTimelineView()
    tl_view.setTickFrameNumbersVisible(not tl_view.isTickNumbersVisible())
    tl_view.updateInternal()


def on_style_compact_toggled():

    tl_view = revlens.TL_CNTRL.getFocusedTimelineView()
    tm = tl_view.trackManager()
    for x in range(tm.trackCount()):

        track = tm.getTrackByIndex(x + 1)
        track.setButtonsVisible(False, False)

        if track.trackTypeName() == 'Media':
            track.setWaveformVisible(False)
            track.getButtonByName('thumbnail').setToggled(False)

            track.setThumbnailVisible(False)
            track.getButtonByName('waveform').setToggled(False)


        # tm.getTrackByIndex(x + 1).setWaveformVisible(False)

    tl_view.updateInternal()


def on_style_full_toggled():

    tl_view = revlens.TL_CNTRL.getFocusedTimelineView()
    tm = tl_view.trackManager()

    for x in range(tm.trackCount()):

        track = tm.getTrackByIndex(x + 1)
        track.setButtonsVisible(True, False)

        if track.trackTypeName() == 'Media':
            track.setWaveformVisible(True)
            track.getButtonByName('thumbnail').setToggled(True)

            track.setThumbnailVisible(True)
            track.getButtonByName('waveform').setToggled(True)

    tl_view.updateInternal()


def _toggle_buttons(buttons_toggled):

    tl_view = revlens.TL_CNTRL.getFocusedTimelineView()
    tm = tl_view.trackManager()

    for x in range(tm.trackCount()):

        track = tm.getTrackByIndex(x + 1)
        track.setButtonsVisible(buttons_toggled, False)

    tl_view.updateInternal()


def on_buttons_show():
    _toggle_buttons(True)

def on_buttons_hide():
    _toggle_buttons(False)


def reset_timeline():
    tl = revlens.TL_CNTRL.getFocusedTimelineView()
    zbar = tl.zoomBar()
    tl.setZoom(1.0)
    tl.setPan(0)

    tl.setPanStart(0)

    zbar.bar().setLeftPos(0)
    zbar.bar().setRightPos(0)
    zbar.btnDragLeft().setXPos(0)
    zbar.btnDragRight().setXPos(0)

    fcount = revlens.CNTRL.session().frameCount()
    tl.setFrameStart(1)
    tl.setFrameEnd(fcount)
    tl.setFocusedOnFrame(False)

    tl.buttonBar().reset()

    tl.update()

    tl.refresh()
    tl.updateIfNecessary()
    tl.updateInternal()

    revlens.cmds.display_message('Timeline Reset')

    return

    # session_framecount = revlens.CNTRL.session().frameCount()
    #
    # Timeline focus
    #
    # tl.setFrameStart(0)
    # tl.setFrameCount(session_framecount, True)
    # 
    # bookends (safe measure?)
    #
    # tl.setInFrame(0, True)
    # tl.setOutFrame(session_framecount, True)



def toggle_input_mode():

    curr_input_mode = revlens.TL_CNTRL.getFocusedTimelineView().timelineInputMode()

    input_mode = TL_MODE_SCRUB
    input_mode_str = 'Scrub'
    if curr_input_mode == TL_MODE_SCRUB:
        input_mode = TL_MODE_EDIT
        input_mode_str = 'Edit'

    revlens.TL_CNTRL.getFocusedTimelineView().setTimelineInputMode(input_mode)

    revlens.cmds.display_message('Timeline: {}'.format(input_mode_str))


def set_scrub_mode():
    revlens.TL_CNTRL.getFocusedTimelineView().setTimelineInputMode(TL_MODE_SCRUB)

def set_edit_mode():
    revlens.TL_CNTRL.getFocusedTimelineView().setTimelineInputMode(TL_MODE_EDIT)


def _frame_to_timecode(frame, framerate):

    mins = int(frame / (float(framerate * 60)))
    frames_minus_mins = frame - (mins * framerate * 60)
    secs = int(frames_minus_mins / float(framerate))
    frames_remaining = int(frame - (secs * framerate))

    result = '00:{}:{}.{}'.format(
        str(mins).zfill(2),
        str(secs).zfill(2),
        frames_remaining
    )

    print('{} (rate {}) -> {}'.format(frame, framerate, result))

    return result


def _gen_thumbnail_with_ffmpeg(input_path, frame_list, output_dir=None, extra=None, framerate=24.0):

    output_dir = output_dir or tempfile.mkdtemp(prefix='revlens_thumbnail_')

    frame = frame_list[0]
    
    timecode = _frame_to_timecode(frame, framerate)
    for frame_ext in revlens.media.MediaLocator.FRAME_FORMATS:
        if input_path.endswith(frame_ext) and os.path.isfile(input_path):
            timecode = '00:00:00.0'

    gamma = ''
    if input_path.endswith('.exr'):
        gamma = ' -gamma 2.2'

    output_file = 'frame.%04d.jpg' # 'frame_{}.jpg'.format(frame)
    output_path_template = os.path.join(output_dir, output_file)
    output_path = os.path.join(output_dir, 'frame.0001.jpg')

    cmd = 'ffmpeg {} -ss {} -i {} -frames:v 1 -f image2 {}'.format(
        gamma, timecode, input_path, output_path_template)

    print(cmd)

    os.system(cmd)

    if os.path.isfile(output_path):
        return output_path


def _gen_thumbnail_with_previewmaker(frame_list, output_dir=None, extra=None, framerate=24.0):

    # TODO FIXME: Move PreviewMakerNode to revlens.MEDIA?
    from rlplug_qtbuiltin import RLQTMEDIA_PreviewMakerNode

    if extra is None:
        extra = {}

    media_node = extra.get('media_node')
    if not media_node:
        media_uuid = extra['media_uuid'].toString()
        media_node = revlens.CNTRL.session().getNodeByUuid(media_uuid)

    if not media_node: # .isValid():
        print("_gen_thumbnail_with_previewmaker(): ERROR: INVALID MEDIA NODE")
        return

    output_dir = output_dir or tempfile.mkdtemp(prefix='revlens_thumbnail_')

    frame = frame_list[0]

    output_file = 'frame.{}.jpg'.format(str(frame).zfill(4))
    output_path = os.path.join(output_dir, output_file)

    preview_node = RLQTMEDIA_PreviewMakerNode(media_node)
    image = preview_node.makePreview(int(frame))

    if image.width() > 0:

        image.save(output_path)
        print('Saved {}'.format(output_path))

        return output_path
    else:
        return ''


def request_thumbnail_gen(input_path, frame_list, output_dir=None, extra=None, framerate=24.0):

    if extra and 'media_uuid' in extra:
        return _gen_thumbnail_with_previewmaker(frame_list, output_dir, extra=extra, framerate=framerate)

    else:
        return _gen_thumbnail_with_ffmpeg(input_path, frame_list, output_dir=output_dir, extra=extra, framerate=framerate)


def startup_menu(timeline_menu):

    print('timeline startup menu: {}'.format(timeline_menu))

    # if revlens.gui._UI.get('menu.rmenus.timeline'):
    #     return

    # Timeline - menu
    #
    # timeline_menu = revlens.GUI.addMenu('Timeline')
    
    style_menu = timeline_menu.addItem('Style', {}, False)
    style_compact_action = style_menu.addAction('Compact')
    style_compact_action.triggered.connect(on_style_compact_toggled)

    style_full_action = style_menu.addAction('Full')
    style_full_action.triggered.connect(on_style_full_toggled)

    buttons_menu = timeline_menu.addItem('Buttons', {}, False)
    show_buttons_action = buttons_menu.addAction('Show')
    show_buttons_action.triggered.connect(on_buttons_show)

    hide_buttons_action = buttons_menu.addAction('Hide')
    hide_buttons_action.triggered.connect(on_buttons_hide)

    toggle_cache_action = timeline_menu.addItem('Cache', {}, True)
    toggle_cache_action.setChecked(True)
    toggle_cache_action.triggered.connect(on_cache_visibility_toggled)    

    toggle_ticks_action = timeline_menu.addItem('Frame Lines', {} , True)
    toggle_ticks_action.setChecked(False)
    toggle_ticks_action.triggered.connect(on_tick_lines_visibility_toggled)
    
    toggle_ticks_action = timeline_menu.addItem('Frame Numbers', {}, True)
    toggle_ticks_action.setChecked(False)
    toggle_ticks_action.triggered.connect(on_tick_numbers_visibility_toggled)
    
    '''
    toggle_thumb_action = timeline_menu.addAction('Thumbnail')
    toggle_thumb_action.setCheckable(True)
    toggle_thumb_action.setChecked(True)
    toggle_thumb_action.triggered.connect(self.on_thumb_visibility_toggled)
    '''

    timeline_menu.addSeparator()

    reset_timeline_action = timeline_menu.addItem('Reset', {}, False)
    reset_timeline_action.triggered.connect(reset_timeline)

    # revlens.gui._UI['menu.rmenus'].append(timeline_menu)
    # revlens.gui._UI['menu.rmenus.timeline'] = timeline_menu


def startup():

    # startup_menu() # TODO FIXME: MOVE TO Timeline widget

    from revlens.keyboard import KeyBinding

    KeyBinding.register(
        'e',
        'revlens.gui.timeline.toggle_input_mode',
        'Toggle Timeline Input Mode'
    )


def request_remove_item_at_current_frame(track):

    dialog = ConfirmDialog.create(
        'Delete Item',
        'Delete Item - Are you sure?',
        accept_label='Delete'
    )

    def _on_accept(md=None):
        curr_frame = revlens.cmds.get_current_frame_num()
        track.removeNodeByFrame(curr_frame)

    dialog.accept.connect(_on_accept)



def request_rename_track(track):

    dialog = InputDialog.create(
        'Rename Track',
        'New Name:',
        accept_label='Rename'
    )

    def _on_accept(md):
        track.setName(md['value'])
        track.update()

    dialog.accept.connect(_on_accept)


def request_delete_track(track_uuid):

    dialog = ConfirmDialog.create(
        'Delete Track',
        'Delete Track - Are you sure?',
        accept_label='Delete'
    )

    def _on_accept(md=None):
        revlens.CNTRL.session().deleteTrack(track_uuid, True)

    dialog.accept.connect(_on_accept)



def request_clear_track(track):

    dialog = ConfirmDialog.create(
        'Clear Track',
        'Clear Track - Are you sure?',
        accept_label='Clear'
    )

    def _on_accept(md=None):
        track.clear()

    dialog.accept.connect(_on_accept)