#!/usr/bin/env python3

import os
import sys
import pprint
import argparse

inst_dir = os.path.abspath(__file__)
for _ in range(3):
    inst_dir = os.path.dirname(inst_dir)

python_dirname = 'python{}.{}'.format(sys.version_info.major, sys.version_info.minor)
core_lib_dir = os.path.join(inst_dir, 'core', 'lib', python_dirname, 'site-packages')
revlens_lib_dir = os.path.join(inst_dir, 'revlens', 'lib', python_dirname, 'site-packages')

sys.path.insert(0, revlens_lib_dir)
sys.path.insert(0, core_lib_dir)

import rlp.util.env

def start(global_dir):

    revlens_bin = 'rlp_gui'
    if os.name == 'nt':
        revlens_bin = 'rlp_gui.exe'
        
    revlens_bin = os.path.join(global_dir, 'bin', revlens_bin)
    print(revlens_bin)

    if os.name == 'nt':

        if os.getenv('REVLENS_PROCDUMP'):
            
            # procdump -t for exit
            pdump_cmd = r'start C:\Users\justi\Downloads\Procdump\procdump.exe -t rlp_gui.exe'
            print(pdump_cmd)
            os.system(pdump_cmd)
        

        exec_cmd = '{} {}'.format(revlens_bin, ' '.join(sys.argv[1:]))

        print(exec_cmd)
        os.system(exec_cmd)
        # os.system(r'C:\Users\justi\Downloads\Dependencies_x64_Release\DependenciesGui.exe')

    else:
        os.execvpe(revlens_bin, sys.argv, os.environ)


def start_batch(global_dir, script):

    print('starting batch mode')
    revlens_batch_bin = os.path.join(global_dir, 'bin', 'revlens_batch.bin')
    print(revlens_batch_bin)
    # os.chdir(os.path.join(global_dir, 'bin'))

    revlens_batch_cmd = '{exe} {script}'.format(exe=revlens_batch_bin, script=script)
    print(revlens_batch_cmd)
    os.system(revlens_batch_cmd)



def main():

    '''
    parser = argparse.ArgumentParser('revlens')
    parser.add_argument('--noaudio', action='store_true', default=False, help='disable audio')
    parser.add_argument('--batch', action='store_true', default=False, help='batch mode')
    parser.add_argument('--script', help='run script on startup')
    parser.add_argument("media_list", help='media list', nargs='*')
    args = parser.parse_args()
    '''
    
    global_dir = rlp.util.env.setup_env()

    '''
    if args.batch:
        start_batch(global_dir, args.script)

    else:
    '''
    start(global_dir)


if __name__ == '__main__':
    main()