#!/usr/bin/env python

import os
import sys


EXIT_BAD_START_DIR = 1
EXIT_BAD_COPY = 2

def main():

    # first check that we're at the top level of the superproject
    #
    currdirs = os.listdir('.')
    if '.git' not in currdirs:
        print('ERROR: run from top of project')
        sys.exit(EXIT_BAD_START_DIR)


    dest_dir = sys.argv[1]
    print('Copying to: {}'.format(dest_dir))

    top_dir = os.path.abspath('.') + '/'
    start_dir = os.path.join(top_dir, 'src/')

    print('Start dir: {}'.format(start_dir))
    for root, dirs, files in os.walk(start_dir):
        reldir = root.replace(top_dir, '')
        
        if '.git' in root:
            print('Skipping {}'.format(root))
            continue


        for file_entry in files:
            relpath = os.path.join(reldir, file_entry)
            dest_abspath = os.path.join(dest_dir, 'src', relpath)
            file_dest_dir = os.path.dirname(dest_abspath)
            if not os.path.isdir(file_dest_dir):
                print('Creating {}'.format(file_dest_dir))
                os.makedirs(file_dest_dir)

            print('{} -> {}'.format(relpath, dest_abspath))
            cp_cmd = 'cp {} {}'.format(relpath, dest_abspath)
            exitcode = os.system(cp_cmd)
            if exitcode != 0:
                print('Error: copy failed, aborting')
                sys.exit(EXIT_BAD_COPY)


if __name__ == '__main__':
    main()
