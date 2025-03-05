#!/usr/bin/env python3

import os
import sys


def main():

    db_path = sys.argv[1]

    # cmd = 'sqlite3 {} "select evt_r_rowid, evt_name, evt_uuid from edb_event;"'.format(db_path)
    cmd = 'sqlite3 {} "select evt_r_rowid, evt_name, evt_data from edb_event;"'.format(db_path)
    print(cmd)
    print('')

    os.system(cmd)


if __name__ == '__main__':
    main()