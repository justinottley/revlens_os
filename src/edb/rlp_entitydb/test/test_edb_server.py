
import json

from rlp_entitydb.client import EdbClient

def main():

    ws_url = 'ws://127.0.0.1:45303'
    tc = EdbClient(ws_url)
    tc.connect()

    r = tc.find('Sequence', [], ['uuid'])
    print(r)


if __name__ == '__main__':
    main()