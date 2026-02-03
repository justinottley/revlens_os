
import json

import rlp.core as RlpCore
import rlp.gui as RlpGui
import rlp.util as RlpUtil
import rlp.util.convert as rlp_util_convert # ios compat

import revlens
import revlens_prod


class StreamView(RlpGui.GUI_ItemBase):

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self._streamListCallback = RlpCore.CoreNet_RPCFuture()
        self._streamListCallback.resultReady.connect(self.onStreamListReady)

        self.gridView = RlpGui.GUI_GRID_View(self, None)
        self.gridView.setPos(10, 0)

        gridToolbar = self.gridView.toolbar()

        self.refreshButton = RlpGui.GUI_SvgButton(':feather/refresh-cw.svg', self, 20)
        self.refreshButton.setToolTipText('Reload Stream List')
        self.refreshButton.buttonPressed.connect(self.onRefreshRequested)

        self.loadButton = RlpGui.GUI_IconButton('', gridToolbar, 20, 4)
        self.loadButton.setText('Load..')
        self.loadButton.setOutlined(True)
        self.loadButton.buttonPressed.connect(self.onLoadStreamRequested)

        self.statusText = RlpGui.GUI_Label(self, '')

        gridToolbar.layout().addItemAtStart(self.refreshButton, 4)
        gridToolbar.layout().addItem(self.loadButton, 0)
        gridToolbar.layout().addItem(self.statusText, 5)

        colModel = self.gridView.colModel()

        colModel.addCol(
            {
                'col_name': 'username',
                'display_name': 'Username',
                'col_type': 'str',
                'metadata': {},
                'width': 120
            },
            -1
        )
        colModel.addCol({
                'col_name': 'hostname',
                'display_name': 'Hostname',
                'col_type': 'str',
                'metadata': {},
                'width': 140
            },
            -1
        )
        colModel.addCol({
                'col_name': 'screen',
                'display_name': 'Screen',
                'col_type': 'str',
                'metadata': {},
                'width': 300
            },
            -1
        )
        colModel.addCol({
                'col_name': 'opsys',
                'display_name': 'OpSys',
                'col_type': 'str',
                'metadata': {},
                'width': 120
            },
            -1
        )

        self.gridView.updateHeader()
    
        self.initStreamList()


    @property
    def streamCntrl(self):
        return revlens_prod.CNTRL_STREAMINGCONTROLLER_OBJ


    def initStreamList(self):
        self.statusText.setText('Getting stream list..')

        revlens.CNTRL.getServerManager().siteClientSendCall('stream.list',
            [], {}, {}, self._streamListCallback)


    def onRefreshRequested(self, md=None):
        self.initStreamList()


    def onStreamListReady(self, msgObj):
        print('STREAM LIST READY')
        import pprint
        pprint.pprint(msgObj)

        if 'result' not in msgObj or 'result' not in msgObj['result']:
            self.statusText.setText('ERROR: Invalid return for stream list')
            return

        result = msgObj['result']['result'] # FFS
        if not result:
            return

        gridDataList = []
        for streamInfo in json.loads(result)['items']:
            encodedName = streamInfo['name']
            encodedName = encodedName.replace('.', '=')

            streamMd = rlp_util_convert.jsonLoads(rlp_util_convert.b64decode(encodedName))
            pprint.pprint(streamMd)

            gridDataList.append({
                'username': streamMd['user'],
                'hostname': streamMd['host'],
                'screen': '{} : {}x{}'.format(streamMd['screen'], streamMd['w'], streamMd['h']),
                'encoded_name': streamInfo['name'],
                'data': streamMd,
                'opsys': streamMd.get('os')
            })

        self.statusText.setText('')
        self.gridView.setModelDataList(gridDataList)


    def onLoadStreamRequested(self, md):
        selRow = self.gridView.fullSelectionValues()
        if len(selRow) != 1:
            return

        selRow = selRow[0]

        ag = RlpUtil.APPGLOBALS.globals()
        streamUrl = 'rtsp://{}:8554/{}'.format(
            ag['edb.site_hostname'],
            selRow['encoded_name']
        )

        streamInfo = selRow['data']
        print(streamUrl)

        revlens.CNTRL.emitNoticeEvent('video.stream_load_requested', streamInfo)
        self.streamCntrl.loadStream(streamUrl, streamInfo)


    def onParentSizeChangedItem(self, width, height):
        self.setWidth(width)
        self.setHeight(height)
