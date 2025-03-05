import os
import sys
import json
import pprint
import logging

from rlp.core import rlp_logging

rlp_logging.basic_config(logging.VERBOSE)
import revlens

from revlens.media.builder.session_builder import SessionIO, SingleFileSessionIO, SplitFileSessionIO


print 'huh?'

def test_write_single():

    sess = SingleFileSessionIO()
    sess.read('example.rls')

    sess.write('test.rls')

def test_write_split():

    sess = SingleFileSessionIO()
    sess.read('example.rls')


    so = SplitFileSessionIO()
    so.data = sess.data
    so.write('test.rls', '/tmp/test_split')


    # pprint.pprint(so.data)



def test_read_split():

    so = SplitFileSessionIO()
    so.read('/tmp/test_split/test.rls')

    pprint.pprint(so.data)


def test_session():

    print 'test_session'

    sess = SessionIO()
    sess.add_track('Test Track')


    pprint.pprint(sess.data)

    writer = SingleFileSessionWriter(sess)
    writer.flush('out.rls')

if __name__ == '__main__':

    test_name = sys.argv[1]

    print test_name

    eval('test_{}'.format(test_name))()

