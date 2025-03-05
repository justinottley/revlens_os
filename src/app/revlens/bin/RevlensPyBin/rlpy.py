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



def main():

    global_dir = rlp.util.env.setup_env(True)
    print(global_dir)

    os.execvpe('/bin/bash', sys.argv, os.environ)


if __name__ == '__main__':
    main()