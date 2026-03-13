#!/usr/bin/env python

import os
import sys
import tempfile
import argparse

REVLENS_LICENSE_SERVER_PORT = 7510

revlens_lib_dir = os.path.join(
    os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
    'lib', 'python2.7', 'site-packages'
)

sys.path.insert(0, revlens_lib_dir)

import rlp.util.env


def main():

    parser = argparse.ArgumentParser()
    parser.add_argument('--localhost', action='store_true', help='force starting on localhost')
    parser.add_argument('--hostname', help='force use this hostname')

    args = parser.parse_args()

    webapp_args = ''
    if args.localhost:
        webapp_args += ' --localhost '

    elif args.hostname:
        webapp_args += ' --use_ip {}'.format(args.hostname)


    rlp.util.env.setup_env()

    webapp_exec_path = os.path.join(
        rlp.util.env.INST_DIR,
        'web2py',
        'global',
        'bin',
        'web2py_run')

    cmd = '{exec_path} {args} --port {port} restart'.format(
        exec_path=webapp_exec_path,
        args=webapp_args,
        port=REVLENS_LICENSE_SERVER_PORT)

    print cmd
    os.system(cmd)

if __name__ == '__main__':
    main()



'''
VC = '/software/thirdbase/180601/inst/{plat}/scaffold/global/bin/vc_wrapper'

def main():


    bin_dir = os.path.abspath(os.path.dirname(__file__))
    inst_dir = os.path.dirname(os.path.dirname(os.path.dirname(bin_dir)))
    platform = os.path.basename(inst_dir)

    # print bin_dir
    # print inst_dir

    print 'Platform: {p}'.format(p=platform)

    vc_cmd = VC.format(plat=platform)

    # print vc_cmd

    temp_fd, temp_filename = tempfile.mkstemp(prefix='tmp_revlens_run_', suffix='.sh')
    webapp_exec = os.path.join(bin_dir, 'web2py_run')
    cmd = '{wexec} restart'.format(wexec=webapp_exec)

    os.write(temp_fd, '#!/bin/sh\n\n')
    os.write(temp_fd, 'source {vc_cmd} init {inst_dir}/vc_config_v001.yaml\n'.format(
        vc_cmd=vc_cmd, inst_dir=inst_dir))
    os.write(temp_fd, '{cmd}\n'.format(cmd=cmd))
    
    os.close(temp_fd)

    os.chmod(temp_filename, 0777)
    print temp_filename
    print cmd

    os.system(temp_filename)



if __name__ == '__main__':
    main()
'''
