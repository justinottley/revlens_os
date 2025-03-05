#!/usr/bin/env python3

import os
import sys
import pprint
import argparse
import platform

python_dirname = 'python{}.{}'.format(sys.version_info.major, sys.version_info.minor)
revlens_lib_dir = os.path.join(
    os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
    'lib', python_dirname, 'site-packages'
)

print(revlens_lib_dir)

sys.path.insert(0, revlens_lib_dir)

import rlp.util.env

def run(global_dir, args):

    exec_cmd = args[0]
    if os.name == 'nt':
        exec_cmd += '.exe'
        
    
    # exec_path = os.path.join(global_dir, 'bin', exec_cmd)
    exec_path = exec_cmd
    # print(exec_path)


    if os.name == 'nt':

        if os.getenv('REVLENS_PROCDUMP'):
            
            # procdump -t for exit
            pdump_cmd = r'start C:\Users\justi\Downloads\Procdump\procdump.exe -t {}'.format(exec_cmd)
            print(pdump_cmd)
            os.system(pdump_cmd)


        os.system(exec_cmd)

    else:
        print(sys.argv)
        os.execvpe(exec_path, sys.argv[1:], os.environ)



def main():

    parser = argparse.ArgumentParser('run_wrapper')
    parser.add_argument('--min', default=False, action='store_true')
    parser.add_argument('exec_cmd', nargs='*', help='exec cmd')
    args = parser.parse_args()

    full = True
    if args.min:
        full=False

    global_dir = rlp.util.env.setup_env(full=full)

    print(args.exec_cmd)

    # args = sys.argv[1:]
    # print(args)
    run(global_dir, args.exec_cmd)


if __name__ == '__main__':
    main()