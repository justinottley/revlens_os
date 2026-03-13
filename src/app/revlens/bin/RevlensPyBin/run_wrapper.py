#!/usr/bin/env python3

import os
import sys
import pprint
import argparse
import platform

revlens_lib_dir = os.path.join(
    os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
    'lib', 'python3.7', 'site-packages'
)

sys.path.insert(0, revlens_lib_dir)

import rlp.util.env

def start_gui(global_dir, args):

    exec_cmd = args.exec_cmd
    if os.name == 'nt':
        exec_cmd += '.exe'
        
    exec_path = os.path.join(global_dir, 'bin', exec_cmd)
    print(exec_path)

    if os.name == 'nt':

        if os.getenv('REVLENS_PROCDUMP'):
            
            # procdump -t for exit
            pdump_cmd = r'start C:\Users\justi\Downloads\Procdump\procdump.exe -t {}'.format(exec_cmd)
            print(pdump_cmd)
            os.system(pdump_cmd)


        os.system(exec_cmd)

    else:
        os.execvpe(exec_path, sys.argv, os.environ)



def main():

    parser = argparse.ArgumentParser('run_wrapper')
    parser.add_argument('exec_cmd', help='exec cmd')
    args = parser.parse_args()

    global_dir = rlp.util.env.setup_env()

    start_gui(global_dir, args)


if __name__ == '__main__':
    main()