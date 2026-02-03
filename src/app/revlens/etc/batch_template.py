import os
import sys
import json
import tempfile


import rlp.gui as RlpGui
import revlens
import revlens.media

_CONTAINER_FORMATS = ['mov', 'mp4', 'avi', 'm4v', 'mj2']

SCENE = r'__SCENE__'
START_FRAME = __START_FRAME__
END_FRAME = __END_FRAME__

OUTPUT_PATH = '__OUTPUT_PATH__' # 'session_export.mov'
OUTPUT_FORMAT = '__OUTPUT_FORMAT__'

OUTPUT_WIDTH = __OUTPUT_WIDTH__
OUTPUT_HEIGHT = __OUTPUT_HEIGHT__

FRAME_PREFIX = '__FRAME_PREFIX__'
FRAME_FORMAT = '__FRAME_FORMAT__'

CREATE_EDB_MEDIA = __CREATE_EDB_MEDIA__
MEDIA_TITLE = '__MEDIA_TITLE__'
COMMENT = '__COMMENT__'

TMP_DIR = tempfile.mkdtemp(prefix='revlens_export_')

EXPORT_DATA = {}

frames_dir = os.path.join(TMP_DIR, 'frames')
os.mkdir(frames_dir)

media_dir = os.path.join(TMP_DIR, 'media')
os.mkdir(media_dir)


revlens.media.load_media_list([SCENE])

revlens.GUI.initPresentationWindow(False)
revlens.GUI.setPresentationWindowSize(OUTPUT_WIDTH, OUTPUT_HEIGHT)

v = revlens.GUI.presentationWindow()

curr_frame = 0


def _get_frame_path(rframe):

    global frames_dir
    global FRAME_PREFIX
    global FRAME_FORMAT

    frame_path = os.path.join(
        frames_dir,
        '{}.{}.{}'.format(
            FRAME_PREFIX,
            str(rframe - START_FRAME + 1).zfill(4),
            FRAME_FORMAT
        )
    )

    return frame_path


def save_thumbnail_data():

    print('save_thumbnail_data')

    import os
    import base64

    from rlp import QtCore, QtGui

    global _get_frame_path
    global START_FRAME, END_FRAME, EXPORT_DATA

    frame_count = END_FRAME - START_FRAME + 1
    midframe = int(frame_count / 2)

    frame_path = _get_frame_path(midframe)

    img = QtGui.QImage(QtCore.QString(frame_path)).scaledToHeight(180)
    temp_fd, temp_path = tempfile.mkstemp(prefix='export_thumbnail_', suffix='.png')
    os.close(temp_fd)

    img.save(temp_path)

    thumbnail_data = base64.b64encode(open(temp_path, 'rb').read()).decode('utf-8')

    EXPORT_DATA['thumbnail_data'] = thumbnail_data
    os.remove(temp_path)


def create_media_entity():

    from rlp_prod.site.client_ws import SiteWsClient

    global MEDIA_TITLE, COMMENT, EXPORT_DATA, START_FRAME, END_FRAME
    global shutdown

    edbc = SiteWsClient.instance()


    def _media_created(result):
        print(result)
        EXPORT_DATA['media_uuid'] = result
        shutdown()


    # f_output_path = os.path.join(
    #     OUTPUT_PATH,
    #     '{}.{}'.format(FRAME_PREFIX, OUTPUT_FORMAT)
    # )
    # file_uuid = '{' + os.path.basename(os.path.dirname(OUTPUT_PATH)) + '}'
    # file_size = os.stat(f_output_path).st_size

    frame_count = END_FRAME - START_FRAME + 1

    media_payload = {
        'title': MEDIA_TITLE,
        'comment': COMMENT,
        'thumb_frame_data': EXPORT_DATA['thumbnail_data'],
        'frame_count': frame_count
        # 'rfs_uuid': file_uuid,
        # 'size': file_size
    }

    edbc.call(_media_created, 'edbc.create', 'Media', 
        MEDIA_TITLE, media_payload, '').run()




def create_media():
    global media_dir
    global frames_dir
    global FRAME_PREFIX
    global OUTPUT_PATH
    global OUTPUT_FORMAT
    global _CONTAINER_FORMATS
    global EXPORT_DATA

    print('CREATING MEDIA')

    if OUTPUT_FORMAT in _CONTAINER_FORMATS:

        if os.path.isfile(OUTPUT_PATH):
            os.remove(OUTPUT_PATH)

        media_path = os.path.join(
            media_dir,
            '{}.{}'.format(FRAME_PREFIX, OUTPUT_FORMAT)
        )

        f_output_path = os.path.join(
            OUTPUT_PATH,
            '{}.{}'.format(FRAME_PREFIX, OUTPUT_FORMAT)
        )

        ffmpeg_cmd = 'ffmpeg -r 24 -i {}/{}.%04d.png -vcodec mjpeg -pix_fmt yuvj420p -q:v 2 {}'.format(
            frames_dir,
            FRAME_PREFIX,
            media_path
        )
        print(ffmpeg_cmd)
        os.system(ffmpeg_cmd)

        file_size = os.stat(media_path).st_size
        EXPORT_DATA['file_size'] = file_size
        EXPORT_DATA['output_path'] = f_output_path

        print('{} -> {}'.format(media_path, f_output_path))
        cmd = 'cp {} {}'.format(media_path, f_output_path)
        print(cmd)
        os.system(cmd)

    else:

        for file_entry in os.listdir(frames_dir):
            tmp_path = os.path.join(frames_dir, file_entry)
            dest_path = os.path.join(OUTPUT_PATH, file_entry)

            print('{} -> {}'.format(tmp_path, dest_path))
            cmd = 'cp {} {}'.format(tmp_path, dest_path)
            # os.rename(tmp_path, dest_path)
            os.system(cmd)




def export_done():

    import traceback
    print('EXPORT DONE!!!')

    global CREATE_EDB_MEDIA
    global create_media, create_media_entity, save_thumbnail_data, shutdown

    try:
        save_thumbnail_data()
        create_media()
        if CREATE_EDB_MEDIA:
            create_media_entity()

        else:
            shutdown()

    except:
        print(traceback.format_exc())



def shutdown():

    print('Writing output info')

    global frames_dir
    global OUTPUT_PATH, EXPORT_DATA

    output_info_path = os.path.join(OUTPUT_PATH, 'export.json')
    with open(output_info_path, 'w') as wfh:
        wfh.write(json.dumps(EXPORT_DATA, indent=2))
        print('Wrote {}'.format(output_info_path))


    for root, dirs, files in os.walk(frames_dir):
        for file_entry in sorted(files):
            file_path = os.path.join(root, file_entry)
            print('Cleaning {}'.format(file_path))
            os.remove(file_path)

    print('Cleaning {}'.format(frames_dir))
    os.rmdir(frames_dir)

    print('SHUTTING DOWN')

    import revlens
    revlens.CNTRL.forceQuit()



def _on_frame_ready():

    import revlens

    global v
    global curr_frame
    global START_FRAME
    global END_FRAME
    global OUTPUT_WIDTH, OUTPUT_HEIGHT
    global _get_frame_path
    global export_done

    rframe = revlens.CNTRL.currentFrameNum()
    frame_path = _get_frame_path(rframe)
    
    i = v.toImage()

    if ((i.width() != OUTPUT_WIDTH) or (i.height() != OUTPUT_HEIGHT)):
        print('WARNING: WINDOW SIZE MISMATCH, resizing: {}x{}'.format(OUTPUT_WIDTH, OUTPUT_HEIGHT))

        revlens.GUI.setPresentationWindowSize(OUTPUT_WIDTH, OUTPUT_HEIGHT)
        revlens.CNTRL.gotoFrame(rframe, True, True, False)
        return



    # print('{}: {} x {}'.format(curr_frame, i.width(), i.height()))

    i.save(frame_path)
    print('Wrote {}'.format(frame_path))

    if rframe == END_FRAME:
        print('Done')
        export_done()
        curr_frame = rframe
        return


    elif curr_frame != rframe:
        curr_frame = rframe
        revlens.CNTRL.gotoFrame(rframe + 1, True, True, False)



v.frameReady.connect(_on_frame_ready)
if os.name == 'nt':
    v.show()

else:
    v.showMinimized()

revlens.CNTRL.gotoFrame(START_FRAME, True, True, False)

