#!/usr/bin/env python3

import os
import json
import base64
import pprint
import socket
import platform
import argparse
import subprocess

FFMPEG_BASE_CMD = '''/usr/bin/ffmpeg -thread_queue_size 1024 -video_size {} -framerate 18 -f x11grab -thread_queue_size 1024 -i :100.0+{} -pes_payload_size 0 -c:v h264_nvenc -profile:v high -rc:v vbr -cq:v 19 -b:v 0 -maxrate 6M -bufsize 12M -pix_fmt rgb0 -g 90 -strict_gop 1 -preset p3 -tune ull -2pass 0 -b_adapt 0 -no-scenecut 1 -nonref_p 1 -forced-idr 1 -zerolatency 1 -flags +global_header -f mpegts -rtsp_transport tcp -f rtsp rtsp://{}:8554/{}'''
FFMPEG_BASE_CMD_MACOS = '''ffmpeg -f avfoundation -i "{}" -vf "scale={}" -r 18 -c:v h264_videotoolbox -profile:v high -b:v 0 -maxrate 6M -bufsize 12M -g 90 -realtime 1 -an -flags +global_header -f mpegts -rtsp_transport tcp -f rtsp rtsp://{}:8554/{}'''


def main():
    
    parser = argparse.ArgumentParser('')
    parser.add_argument('--screen_idx', dest='screen_idx')
    parser.add_argument('--screen_name', dest='screen_name', default='')
    parser.add_argument('--resolution', dest='resolution', default='1920x1080')
    parser.add_argument('--position', dest='position', default='0,0')
    parser.add_argument('--server', dest='server', help='mediamtx server')
    parser.add_argument('--dry_run', action='store_true', default=False)

    args = parser.parse_args()

    resolution = args.resolution
    res_parts = resolution.split('x')

    # Don't clamp resolution
    #
    # if int(res_parts[0]) > 1920:
    #     res_parts[0] = 1920
    #
    # if int(res_parts[1]) > 1080:
    #     res_parts[1] = 1080

    resolution = '{}x{}'.format(res_parts[0], res_parts[1])

    hostname = os.getenv('HOSTNAME')
    opsys = 'Linux'
    if platform.system() == 'Darwin' and not hostname:
        hostname = subprocess.getoutput('echo $HOSTNAME') # ?????
        opsys ='macOS'


    # encode metadata into stream name
    mdata = {
        'host': hostname,
        'user': os.getenv('USER'),
        'screen': args.screen_name,
        'w': int(res_parts[0]),
        'h': int(res_parts[1]),
        'ip': socket.gethostbyname(socket.gethostname()),
        'os': opsys
    }

    pprint.pprint(mdata)
    stream_name = base64.b64encode(json.dumps(mdata).encode('utf-8')).decode('utf-8')

    # fixup base64 padding for mediamtx

    stream_name = stream_name.replace('=', '.')
    print(stream_name)

    cmd = FFMPEG_BASE_CMD.format(resolution, args.position, args.server, stream_name)
    if platform.system() == 'Darwin':
        cmd = FFMPEG_BASE_CMD_MACOS.format(args.screen_idx, resolution, args.server, stream_name)


    print(cmd)

    if not args.dry_run:
        os.system(cmd)

    print('Running')



if __name__ == '__main__':
    main()
