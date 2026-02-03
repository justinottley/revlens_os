#!/usr/bin/env python

import os
import sys
import pprint
import logging
import hashlib
import argparse
import traceback


from rlp.core import rlp_logging
import rlp.core.process.cmds

rlogger = logging.getLogger('rlp.{}'.format(__name__))

EXIT_SUCCESS = 0

def init_logging(loglevel):
    rlp_logging.basic_config(rlogger, loglevel)
    
    
def run_ps(executable):
    
    ps_result = rlp.core.process.cmds.get_process_list(executable)
    
    for entry_info in ps_result:
        
        print '{exc} {pid}'.format(exc=entry_info['executable'], pid=entry_info['pid'])
        
        if entry_info['args']:
            arg_str = entry_info['args']
            arg_list = arg_str.split()
            for arg_entry in arg_list:
                print '  {a}'.format(a=arg_entry)
                
        print ''
        
        
def main():
    
    parser = argparse.ArgumentParser('Process listing - shows processes with commandline arguments')
    parser.add_argument('--loglevel', dest='loglevel', help='loglevel', default=logging.INFO)
    parser.add_argument('executable', help='Executable name. On Windows - e.g., "python.exe"')
    
    args = parser.parse_args()
    
    init_logging(args.loglevel)
    run_ps(args.executable)
    
    
if __name__ == '__main__':
    main()
