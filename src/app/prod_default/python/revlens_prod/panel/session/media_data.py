

import pprint

from rlp import QtCore

from rlp.core.net.websocket import RlpClient

import revlens

def getItemChoiceWidgetInfo():

    return {
        'menu_widget': 'grid',
        'menu_widget.width': 140,
        'menu_widget.height': 500,
        'menu_widget.colmodel': [
            {
                'col_name': 'name',
                'col_type': 'str',
                'display_name': 'Name',
                'width': 140,
                'metadata': {}
            }
            # { # Thumbnail data not available yet (for example shot)
            #     'col_name': 'thumb',
            #     'col_type': 'image_b64',
            #     'display_name': 'Thumb',
            #     'width': 80,
            #     'metadata': {}
            # }
        ]
    }


def getChoiceWidgetInfo(numDisplayName):

    choiceWidgetInfo = {
        'menu_widget': 'grid',
        'menu_widget.width': 500,
        'menu_widget.height': 500,
        'menu_widget.colmodel': [
            {
                'col_name': 'num',
                'col_type': 'str',
                'display_name': numDisplayName,
                'width': 60,
                'metadata': {}
            },
            {
                'col_name': 'image_path',
                'col_type': 'image_path',
                'display_name': 'Thumb',
                'width': 80,
                'metadata': {}
            },
            {
                'col_name': 'dept',
                'col_type': 'str',
                'display_name': 'Department',
                'width': 100,
                'metadata': {}
            },
            {
                'col_name': 'description',
                'col_type': 'str',
                'display_name': 'Description',
                'width': 260,
                'metadata': {}
            }
        ]
    }

    return choiceWidgetInfo


class TrackMediaData(QtCore.QObject):

    def __init__(self, track):
        QtCore.QObject.__init__(self)

        self.rowUuidMap = {}

        self.mediaListReady = QtCore.PY_Signal(self)
        self.mediaDataReady = QtCore.PY_Signal(self)

        self.track = track
        self.track.mediaDataReady.connect(self.onMediaDataReady)


    def refresh(self):

        # self.trackLabel.setText(self.track.name())
        itemList = []
        trackUuid = self.track.uuid().toString()
        nodeFrameList = self.track.getNodeFrameList()

        self.rowUuidMap = {}

        rowIdx = 0
        for trackItem in nodeFrameList:

            frameNum = trackItem['frame']
            node = self.track.getNodeByFrame(frameNum)
            nodeProps = node.getProperties()
            nodeUuid = nodeProps['graph.uuid'].toString()
            nodeMd = nodeProps.get('media.metadata', {})


            gridData = {
                'name': nodeProps['media.name'],
                'frame': frameNum,
                'length': nodeProps['media.frame_count'],
                'uuid': nodeUuid,
                'track_uuid': trackUuid,
                'dept': nodeMd.get('dept', ''),
                'version': nodeMd.get('version', ''),
                'metadata': nodeMd,
                'source_info': nodeProps['media.source_info']
            }

            for imageKey in ['media.thumbnail.image_full', 'media.thumbnail.image', 'media.thumbnail.data']:
                if imageKey in nodeProps:
                    gridData['image'] = nodeProps[imageKey]
                    break

            itemList.append(gridData)
            self.rowUuidMap[nodeUuid] = rowIdx
            rowIdx += 1

        self.mediaListReady.emit({'media': itemList})


    def onMediaDataReady(self, evtName, evtInfo):
        
        if evtName != 'thumbnail_ready':
            return

        # We need to get the row index and pass it on..
        #

        incomingUuid = evtInfo['graph.uuid'].toString()
        if incomingUuid in self.rowUuidMap:
            rowIdx = self.rowUuidMap[incomingUuid]
            evtInfo['row_idx'] = rowIdx
            evtInfo['track_uuid'] = self.track.uuid().toString()
            md = {'evt_name': evtName, 'evt_info': evtInfo}
            self.mediaDataReady.emit(md)


    def disconnectPySlots(self):
        self.mediaListReady.disconnectAll()



class TrackMediaWidgetHandler(QtCore.QObject):

    DEPTS = [
        'editorial',
        'layout',
        'animation',
        'lighting'
    ]

    def __init__(self, trackMediaWidget):
        QtCore.QObject.__init__(self)

        self.trackMediaWidget = trackMediaWidget

        shotContextMenuItem = self.trackMediaWidget.contextMenu.item('Context')
        shotContextMenuItem.menu().menuItemSelected.connect(self.onContextMenuItemSelected)

        self.trackMediaWidget.contextMenu.menuItemSelected.connect(self.onContextMenuItemSelected)

        self.grid.modelDataChanged.connect(self.onGridModelDataChanged)
        self.grid.cellWidgetEvent.connect(self.onGridCellWidgetEvent)

        self.initColModel()


    @property
    def edbc(self):
        return RlpClient.instance()

    @property
    def grid(self):
        return self.trackMediaWidget.grid

    @property
    def currTrack(self):
        return self.trackMediaWidget.currTrack

    @property
    def currSelected(self):
        return self.trackMediaWidget.currSelected


    def initColModel(self):

        colModel = self.grid.colModel()

        colModel.addCol({
            "col_name": "_reorder",
            "display_name": "_reorder",
            "col_type": "str",
            "metadata": {},
            "width": 30
        }, -1)

        colModel.addCol({
            "col_name": "name",
            "display_name": "Name",
            "col_type": "str",
            "metadata": {
                'widgets': {
                    'choice': getItemChoiceWidgetInfo()
                }
            },
            "width": 120
        }, -1)

        colModel.addCol({
            "col_name": "image",
            "display_name": "Thumbnail",
            "col_type": "image_b64",
            "metadata": {},
            "width": 80
        }, -1)

        colModel.addCol({
            'col_name': 'dept',
            'display_name': 'Department',
            'col_type': 'str',
            'metadata': {
                'widgets': {
                    'choice': {}
                }
            },
            'width': 120
        }, -1)

        colModel.addCol({
            'col_name' : 'version',
            'display_name': 'Version',
            'col_type': 'str',
            'metadata': {
                'widgets': {
                    'choice': getChoiceWidgetInfo('Version')
                }
            },
            'width': 80
        }, -1)


    def onContextMenuItemSelected(self, md):
        raise Exception('implement in subclass')


    def onGridModelDataChanged(self):
        raise Exception('implement in subclass')
    

    def onGridCellWidgetEvent(self, evtName, evtInfo):
        raise Exception('implement in subclass')