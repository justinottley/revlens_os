#!/usr/bin/env python3

import os
import sys

import rlp.core.process.cmds as rlp_proc_cmds
import rlp.core.path.util as rlp_path_util

def main():

    print('Attempting to shut down previous mediamtx instance..')

    skip_name = os.path.basename(sys.argv[0])
    rlp_proc_cmds.posix_killall('mediamtx', skip_tags=[skip_name])

    bin_path = rlp_path_util.which('mediamtx')
    config_path = '{}.yml'.format(bin_path)

    cmd = 'mediamtx {}'.format(config_path)

    print('')
    print('Running {}'.format(cmd))
    print('')

    os.system(cmd)

if __name__ == '__main__':
    main()