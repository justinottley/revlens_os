

import platform
import subprocess

from rlp import QtGui


def get_macos_video_devices():

    ffmpeg_cmd = 'ffmpeg -f avfoundation -list_devices true -i ""'
    ffmpeg_output = subprocess.getoutput(ffmpeg_cmd)

    result = []

    for line in ffmpeg_output.split('\n'):
        if 'Capture screen' in line:
            line_parts = line.split('[')
            if len(line_parts) == 3:
                screen_idx = line_parts[-1].split(']')[0]
                print('Got Capture Screen index: {}'.format(screen_idx))
                result.append(screen_idx)

    return result


def get_screen_info():

    deviceList = []
    if platform.system() == 'Darwin':
        deviceList = get_macos_video_devices()


    result = []
    screenList = QtGui.QApplication.instance().screens()

    screenIdx = 0
    for screenEntry in screenList:
        geo = screenEntry.geometry()

        # print('GEO: {} {} {} {}'.format(
        #     geo.x(),
        #     geo.y(),
        #     geo.width(),
        #     geo.height()
        # ))

        # ageo = screenEntry.availableGeometry()

        # print('AGEO: {} {} {} {}'.format(
        #     ageo.x(),
        #     ageo.y(),
        #     ageo.width(),
        #     ageo.height()
        # ))

        mdata = {
            'position': '{},{}'.format(geo.x(), geo.y()),
            'resolution': '{}x{}'.format(geo.width(), geo.height()),
            'screen_name': screenEntry.name()
        }

        if deviceList:
            captureIdx = deviceList[screenIdx]
            mdata['screen_idx'] = int(captureIdx)

        else:
            mdata['screen_idx'] = screenIdx

        screenIdx += 1
        result.append(mdata)


    return result



