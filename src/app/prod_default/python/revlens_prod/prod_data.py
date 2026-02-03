
import pprint

from rlp import QtCore
import rlp.util as RlpUtil

from rlp.core.net.websocket import RlpClient

class MockDataSource:

    def __init__(self):
        pass

    def init(self, callback, force=False):
        pass


class MediaData(QtCore.QObject):

    def __init__(self, shotData, values):
        QtCore.QObject.__init__(self)

        # TODO FIXME: shot / seq / show namespace
        cbFormatterCode = shotData.seq.showData.formatter('code')
        _, metadata = cbFormatterCode(values, nameKey='shot')
        values['metadata'] = metadata

        self.__shotData = shotData
        self.__values = values

    @property
    def shot(self):
        return self.__shotData

    @property
    def values(self):
        return self.__values


class ShotData(QtCore.QObject):

    def __init__(self, seqData, values, shotIdx):
        QtCore.QObject.__init__(self)

        self.__seqData = seqData
        self.__values = values

        self.__shotIdx = shotIdx

        cbFormatterCode = seqData.showData.formatter('code')
        name, metadata = cbFormatterCode(values)
        values['metadata'] = metadata

        self.__name = name
        # index by dept, tag? artist?
        self._indexes = {'version': {}}
        self.__mediaList = []


    @property
    def seq(self):
        return self.__seqData

    @property
    def values(self):
        return self.__values

    @property
    def idx(self):
        return self.__shotIdx

    @property
    def name(self):
        return self.__name

    @property
    def mediaList(self):
        return self.__mediaList


    def getMediaByIndex(self, name):
        return self._indexes.get(name, {})


    def loadMedia(self, mediaEntry):

        mediaObj = MediaData(self, mediaEntry)
        self.mediaList.append(mediaObj)
        if 'version' in mediaObj.values:
            mediaVersion = int(mediaObj.values['version'])
            self._indexes['version'][mediaVersion] = mediaObj


    def next(self):
        return self.seq.next(self.idx)

    def prev(self):
        return self.seq.prev(self.idx)


class SequenceData(QtCore.QObject):

    def __init__(self, showData, seqCode, values):
        QtCore.QObject.__init__(self)

        self.__showData = showData
        self.__values = values
        self.__name = seqCode

        self._shotCodeIdxMap = {}
        self._shotIdxMap = {}

        shotIdx = 0
        for entry in values:
            shotCode = entry['code']
            sobj = ShotData(self, entry, shotIdx)
            self._shotCodeIdxMap[shotCode] = sobj
            self._shotIdxMap[shotIdx] = sobj
            shotIdx += 1


    @property
    def showData(self):
        return self.__showData

    @property
    def name(self):
        return self.__name

    @property
    def values(self):
        return self.__values

    def next(self, idx):
        nextIdx = idx + 1
        if nextIdx in self._shotIdxMap:
            return self._shotIdxMap[nextIdx]

    def prev(self, idx):
        prevIdx = idx - 1
        if prevIdx >= 0 and prevIdx in self._shotIdxMap:
            return self._shotIdxMap[prevIdx]

    def getShot(self, shotCode):

        if shotCode in self._shotCodeIdxMap:
            return self._shotCodeIdxMap[shotCode]

    def loadMedia(self, mediaList):

        print('Loading {} media items'.format(len(mediaList)))
        for mediaEntry in mediaList:

            shotCode = mediaEntry.get('shot')
            if not shotCode:
                print('ERROR: INVALID DATA, NO SHOT: {}'.format(shotCode))
                return
            
            # TODO FIXME: sigh... ShotCode object with formatting options?..
            if '/' not in shotCode:
                shotCode = '{}/{}'.format(self.name, shotCode)

            shotObj = self.getShot(shotCode)
            if not shotObj:

                print('ERROR: UNKNOWN SHOT {}'.format(shotCode))
                print(self._shotCodeIdxMap.keys())
                return

            shotObj.loadMedia(mediaEntry)



class ShowData(QtCore.QObject):

    _INSTANCE = None

    def __init__(self):
        QtCore.QObject.__init__(self)

        self._FORMATTERS = {}
        self._DATA_SOURCE = {}

        self._seqs = {}

        # may not be avialable when this object is constructed
        self._projectEntity = None


    @classmethod
    def instance(cls):
        if cls._INSTANCE is None:
            cls._INSTANCE= cls()

        return cls._INSTANCE


    @property
    def edbc(self):
        return RlpClient.instance()


    @property
    def projectId(self):
        return self.projectEntity.get('id')


    @property
    def projectEntity(self):

        if self._projectEntity is not None:
            return self._projectEntity

        ag = RlpUtil.get_app_globals()
        if 'project.info' in ag:
            self._projectEntity = ag['project.info']
            return self._projectEntity
        
        else:
            return {}


    def setFormatter(self, name, callback):
        self._FORMATTERS[name] = callback


    def setDataSource(self, name, obj):
        self._DATA_SOURCE[name] = obj


    def formatter(self, name):
        return self._FORMATTERS.get(name)


    def dataSource(self, name):
        if name not in self._DATA_SOURCE:
            print('WARNING: no data source available for "{}", using a mock'.format(name))
            self._DATA_SOURCE[name] = MockDataSource()

        return self._DATA_SOURCE.get(name)
    

    def loadSequence(self, callback, seqCode, force=False):

        if seqCode in self._seqs and not force:
            callback(self._seqs[seqCode])

        else:

            def _dataReady(result):
                sd = SequenceData(self, seqCode, result)
                self._seqs[seqCode] = sd
                callback(sd)

            self.edbc.ioclient('sg').call(_dataReady,
                'cmds.get_shots_by_sequence',
                self.projectId, seq_code=seqCode).run()


    def getSequence(self, seqCode):
        return self._seqs.get(seqCode)



