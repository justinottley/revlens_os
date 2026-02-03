#!/usr/bin/env python3

import os
import sys
import pprint
import argparse
import platform
import traceback
import subprocess

inst_dir = os.path.abspath(__file__)
for _ in range(3):
    inst_dir = os.path.dirname(inst_dir)

print(inst_dir)

python_dirname = 'python{}.{}'.format(sys.version_info.major, sys.version_info.minor)
core_lib_dir = os.path.join(inst_dir, 'core', 'lib', python_dirname, 'site-packages')
revlens_lib_dir = os.path.join(inst_dir, 'revlens', 'lib', python_dirname, 'site-packages')


sys.path.insert(0, revlens_lib_dir)
sys.path.insert(0, core_lib_dir)

import rlp.util.env

def start(global_dir):

    launcher_bin = 'rlp_launcher.bin'
    if os.name == 'nt':
        launcher_bin += '.exe'

    launcher_path = os.path.join(global_dir, 'bin', launcher_bin)

    os.execvpe(launcher_path, sys.argv, os.environ)

def main():

    global_dir = rlp.util.env.setup_env(full=False)
    start(global_dir)


if __name__ == '__main__':
    main()