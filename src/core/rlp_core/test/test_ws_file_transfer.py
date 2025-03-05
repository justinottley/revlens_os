import os
import base64
import tempfile

import rlp.core
rlp.core._init()


from rlp.core.net.websocket.rlpqt import RlpQtWsClient

CLIENT = None
TFD = None
TPATH = None

SIZE_TOTAL = 0
SIZE_SOFAR = 0
def _on_download_chunk(eresult):

    # print('on_download_chunk: {}'.format(eresult))

    global SIZE_TOTAL
    global SIZE_SOFAR

    global TFD

    result = eresult['result']

    if result['size'] > 0:
        
        os.write(TFD, base64.b64decode(result['data']))

        SIZE_SOFAR += result['size']
        progress = (SIZE_SOFAR / SIZE_TOTAL) * 100

        print('{} - {}%'.format(result['size'], progress))

        CLIENT.call(
            _on_download_chunk,
            'site.download_chunk',
            pos=result['pos'],
            fh=result['fh']
        ).run()

    else:

        os.close(TFD)

        def _on_download_end(eresult):
            print('on download end: {}'.format(eresult))


        print('DOWNLOAD DONE')
        print(TPATH)
        CLIENT.call(
            _on_download_end,
            'site.download_end',
            fh=result['fh']
        ).run()



def _on_download_start(eresult):
    print('got download start')
    print(eresult)

    result = eresult['result']
    if result['fh'] == -1:
        print('could not start')
        return

    global SIZE_TOTAL
    SIZE_TOTAL = result['size_total']

    global TFD, TPATH
    TFD, TPATH = tempfile.mkstemp(prefix='data_', suffix='.mov')

    print(TPATH)
    print(TPATH)

    CLIENT.call(
        _on_download_chunk,
        'site.download_chunk',
        **result
    ).run()



def on_connected():
    print('CLIENT TEST CONNECTED')

    global CLIENT
    CLIENT.call(
        _on_download_start,
        'site.download_start',
        '/home/justinottley/Videos/big_buck_bunny_trailer_480p_1.mov'
    ).run()



def main():

    print('Running')

    global CLIENT
    CLIENT = RlpQtWsClient('ws://127.0.0.1:8003', encrypted=True)
    CLIENT._wsclient.onClientConnected.connect(on_connected)
    CLIENT.connect()

if __name__ == '__main__':
    main()


