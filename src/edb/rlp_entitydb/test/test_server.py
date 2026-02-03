
from rlp_entitydb.client_ws import EdbWsClient


edbc = EdbWsClient('ws://127.0.0.1:8003')
edbc.connect()

def test_find():

    r = edbc.find('Media', [], [])
    print(r)



def main():

    test_find()


if __name__ == '__main__':
    main()

