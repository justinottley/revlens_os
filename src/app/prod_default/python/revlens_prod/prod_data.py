
import pprint

from rlp import QtCore
import rlp.util as RlpUtil

from rlp.core.net.websocket import RlpClient

class MockDataSource:

    def __init__(self):
        pass

    def init(self, callback, force=False):
        pass



class ProdDataObjectBase(QtCore.QObject):

    def __init__(self):
        QtCore.QObject.__init__(self)

    @property
    def edbc(self):
        return RlpClient.instance()



class ShowData(ProdDataObjectBase):

    _INSTANCE = None

    def __init__(self):
        ProdDataObjectBase.__init__(self)

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
    def projectId(self):
        return self.projectEntity.get('id')


    @property
    def projectEntity(self):

        if self._projectEntity is not None:
            return self._projectEntity

        projectInfo = RlpUtil.APPGLOBALS.value('project.info')
        if projectInfo:
            self._projectEntity = projectInfo
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


    def getSequence(self, seqCode, autocreate=False):
        if autocreate and seqCode not in self._seqs:
            seq = ProdDataObjectFactory.makeProdDataObject(
                'sequence', self, seqCode)

            self._seqs[seqCode] = seq

        return self._seqs.get(seqCode)


    def requestDeptVersions(self, callback, requestInfo: dict):

        # TODO FIXME: data classes.......?
        if 'row_data' not in requestInfo or 'metadata' not in requestInfo['row_data']:
            raise Exception('Data error - expert row_data -> metadata')


        rowMd = requestInfo['row_data']['metadata']
        if 'seq' in rowMd and 'shot' in rowMd:

            showDataObj = ProdDataObjectFactory.getShowData()
            seqDataObj = showDataObj.getSequence(rowMd['seq'], autocreate=True)
            shotDataObj = seqDataObj.getShot(rowMd['shot'], autocreate=True)

            shotDataObj.requestDeptVersions(callback, requestInfo)



class SequenceData(ProdDataObjectBase):

    INDEX_TYPE_CONTEXT = 'context'
    INDEX_TYPE_ALL = 'all'

    def __init__(self, showData: ShowData, seqCode: str):
        ProdDataObjectBase.__init__(self)

        self.__showData = showData
        self.__name = seqCode
        self.__values = {}

        self._shotCodeMap = {}
        self._shotIdxMap = {}


    @property
    def showData(self):
        return self.__showData

    @property
    def name(self):
        return self.__name

    @property
    def values(self):
        return self.__values

    def hasIndex(self, indexName):
        return indexName in self._shotIdxMap

    def inIndex(self, indexName, shotObj):

        # slow
        if indexName not in self._shotIdxMap:
            return False
        
        for iShotObj in self._shotIdxMap[indexName].values():

            if iShotObj.name == shotObj.name:
                return True

        return False


    def initShots(self, indexName, shotList):

        print('init shots for index: {} - {}'.format(indexName, len(shotList)))

        if indexName in self._shotIdxMap:
            print('WARNING: Shot Index {} already exists'.format(indexName))

        shotIndexMap = {}
        shotIdx = 0
        for entry in shotList:

            shotCode = entry['code']

            sobj = None
            if shotCode in self._shotCodeMap:
                sobj = self._shotCodeMap[shotCode]
                sobj.idx = shotIdx

            else:
                sobj = ProdDataObjectFactory.makeProdDataObject('shot',
                    seqData=self,
                    shotCode=shotCode,
                    values=entry,
                    shotIdx=shotIdx
                )

                self._shotCodeMap[shotCode] = sobj


            shotIndexMap[shotIdx] = sobj

            shotIdx += 1

        self._shotIdxMap[indexName] = shotIndexMap


    def next(self, idx, indexName='context'):
        nextIdx = idx + 1
        if nextIdx in self._shotIdxMap[indexName]:
            return self._shotIdxMap[indexName][nextIdx]

    def prev(self, idx, indexName='context'):
        prevIdx = idx - 1
        if prevIdx >= 0 and prevIdx in self._shotIdxMap[indexName]:
            return self._shotIdxMap[indexName][prevIdx]

    def getShot(self, shotCode, autocreate=False):
        
        if autocreate and shotCode not in self._shotCodeMap:
            shotDataObj = ProdDataObjectFactory.makeProdDataObject(
                'shot',
                self,
                shotCode=shotCode
            )
            self._shotCodeMap[shotCode] = shotDataObj


        if shotCode in self._shotCodeMap:
            return self._shotCodeMap[shotCode]

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
                return

            shotObj.loadMedia(mediaEntry)


    def requestShots(self, callback: object):
        raise Exception('implement in subclass')


    def requestContextMedia(self, callback: object, dept: str = None, contextSize: int = 1):
        raise Exception('implement in subclass')


class ShotData(ProdDataObjectBase):

    _DEPT_LIST = ['editorial', 'layout', 'sweatbox', 'animation', 'lighting']

    def __init__(self, seqData: SequenceData, shotCode = None, values: dict = None, shotIdx: int = -1):
        ProdDataObjectBase.__init__(self)

        self.__seqData = seqData
        self.__values = values or {}

        if shotCode:
            self.__values['code'] = shotCode

        self.__shotIdx = shotIdx

        cbFormatterCode = seqData.showData.formatter('code')
        name, metadata = cbFormatterCode(self.__values)

        self.__values['metadata'] = metadata

        self.__name = name

        # index by dept, tag? artist?
        self._indexes = {'version': None, 'dept': None}
        self._mediaList = []
        self._deptList = self.__class__._DEPT_LIST[:]


    @property
    def seq(self):
        return self.__seqData

    @property
    def values(self):
        return self.__values

    def _getIdx(self):
        return self.__shotIdx

    def _setIdx(self, value):
        self.__shotIdx = value

    idx = property(_getIdx, _setIdx)


    @property
    def name(self):
        return self.__name

    @property
    def mediaList(self):
        return self._mediaList

    def next(self):
        return self.seq.next(self.idx)

    def prev(self):
        return self.seq.prev(self.idx)

    def copy(self):

        copyValues = self.values.copy() # shallow copy..
        copiedObj = self.__class__(self.seq, self.__values['code'], values=copyValues, shotIdx=self.idx)
        copiedObj._deptList = self._deptList

        return copiedObj


    def getMediaByIndex(self, name, key=None):
        midx = self._indexes.get(name)
        if key and midx:
            return midx.get(key)

        return midx


    def loadMedia(self, mediaEntry):

        if self._indexes['version'] is None:
            self._indexes['version'] = {}

        if self._indexes['dept'] is None:
            self._indexes['dept'] = {}

        mediaObj = ProdDataObjectFactory.makeProdDataObject('media', self, mediaEntry)

        self.mediaList.append(mediaObj)

        # add this object to version and dept indexes if data is available
        #
        if 'version' in mediaObj.values:
            mediaVersion = mediaObj.values['version']
            self._indexes['version'][mediaVersion] = mediaObj

            if 'dept' in mediaObj.values:
                mediaDept = mediaObj.values['dept']
                deptIndex = self._indexes['dept'].setdefault(mediaDept, {})
                deptIndex[mediaVersion] = mediaObj


        return mediaObj


    def requestDeptVersions(self, requestInfo: dict):
        raise Exception('Not implemented, subclass required')


    def getDeptList(self):
        return self._deptList

    def setDeptList(self, deptList):
        self._deptList = deptList


class MediaData(ProdDataObjectBase):

    def __init__(self, shotData: ShotData, values: dict):
        ProdDataObjectBase.__init__(self)

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


class ProdDataObjectFactory:

    _classMap = {
        'shot': ShotData,
        'sequence': SequenceData,
        'media': MediaData,
        'show': ShowData
    }

    def __init__(self):
        raise Exception('Class based usage only, Not meant to be instantiated')


    @classmethod
    def registerProdDataClass(cls, clsName, pcls):
        cls._classMap[clsName] = pcls


    @classmethod
    def makeProdDataObject(cls, clsName, *args, **kwargs):
        pcls = cls._classMap[clsName]
        return pcls(*args, **kwargs)


    @classmethod
    def getShowData(cls):
        return cls._classMap['show'].instance()


    @classmethod
    def getProdDataClass(cls, name):
        if name in cls._classMap:
            return cls._classMap[name]

