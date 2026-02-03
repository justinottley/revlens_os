

'''
class ReviewlistView(RlpGui.GUI_ItemBase):

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.edbc = RlpClient.instance()

        self.reviewLists = None
        self.tableView = TableView(self)
'''



'''
class SequenceView(RlpGui.GUI_ItemBase):

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.tableView = TableView(self)
'''

class MediaPanel1(RlpGui.GUI_ItemBase):

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self._destructed = False

        self.edbc = RlpClient.instance()

        self.project_entity = None
        app_globals = RlpUtil.APPGLOBALS.globals()
        if 'project.info' in app_globals:
            self.project_entity = {'type': 'Project', 'id': app_globals['project.info']['id']}

        self.thumbnail_client = RlpCore.CoreNet_HttpClient()
        self.thumbnail_client.requestReady.connect(self.onThumbRequestReady)

        self.search_client = RlpCore.CoreNet_HttpClient()
        self.search_client.requestReady.connect(self.onSearchRequestReady)

        self.setWidth(parent.width())
        self.setHeight(parent.height())

        self.LOG = logging.getLogger('rlp.{}.{}'.format(self.__class__.__module__, self.__class__.__name__))

        self.view_type = None

        self.searchFilters = []

        self.toolbar_layout = RlpGui.GUI_HLayout(self)
        self.toolbar_layout.addSpacer(10)

        self.refresh_button = RlpGui.GUI_SvgButton(
            ":feather/refresh-cw.svg",
            parent,
            20
        )
        self.refresh_button.buttonPressed.connect(self.refresh)
        self.toolbar_layout.addItem(self.refresh_button, 0)
        self.toolbar_layout.addSpacer(10)

        self.thumb_button = RlpGui.GUI_SvgButton(
            ":feather/grid.svg",
            parent,
            20
        )
        self.thumb_button.buttonPressed.connect(self.onThumbViewRequested)

        self.toolbar_layout.addItem(self.thumb_button, 0)
        self.toolbar_layout.addSpacer(10)

        self.table_button = RlpGui.GUI_SvgButton(
            ":feather/align-justify.svg",
            parent,
            20
        )
        self.table_button.buttonPressed.connect(self.onTableViewRequested)
        
        self.toolbar_layout.addItem(self.table_button, 0)

        # self.toolbar_layout.addDivider(20, 20, 0)

        # self.load_button = RlpGui.GUI_IconButton('', self, 20, 4)
        # self.load_button.setTextYOffset(-1)
        # self.load_button.setText('Load')
        # self.load_button.setOutlined(True)
        # self.load_button.buttonPressed.connect(self.onLoadRequested)
        # self.toolbar_layout.addItem(self.load_button, 0)

        self.toolbar_layout.addDivider(20, 20, 0)

        self.status_label = RlpGui.GUI_Label(self, 'Ready')
        self.status_label.setPos(parent.width() - 100, 5)

        # self.search_bar = RlpGui.GUI_TextEdit(self, 300, 25)
        # self.search_bar.setTempHintText("Search:")
        # self.search_bar.enterPressed.connect(self.onSearchRequested)
        # self.search_bar.scrollArea().setHScrollBarVisible(False)
        # self.toolbar_layout.addItem(self.search_bar, 0)

        self.hierarchy_nav = HierarchyNavWidget(self)
        self.hierarchy_nav.status.connect(self.onNavStatus)
        self.hierarchy_nav.navItemsReady.connect(self.onNavItemsReady)
        self.hierarchy_nav.endSelected.connect(self.onNavItemSelected)
        self.hierarchy_nav.searchRequested.connect(self.onSearchRequested)
        self.hierarchy_nav.initHierarchyMenu()
        self.toolbar_layout.addItem(self.hierarchy_nav, 0)


        self.toolbar_layout.addItem(self.status_label, 0)

        # self.filters = RlpGui.GUI_HLayout(self)
        # self._filters = []

        self.layout = RlpGui.GUI_VLayout(self)
        self.layout.addItem(self.toolbar_layout, 0)
        
        # self.layout.addSpacer(5)
        # self.layout.addItem(self.filters, 10)

        self.thumb_view = ThumbView(self)
        self.layout.addItem(self.thumb_view, 0)


        self.thumb_view.hideItem()

        self.table_view = TableView(self)
        self.layout.addItem(self.table_view, 0)
        self.table_view.hideItem()

        self._curr_view = self.table_view

        self.onParentSizeChangedItem(parent.width(), parent.height())


    def destruct(self):
        self._destructed = True


    def onFieldDataReady(self, md):

        # self.onParentSizeChangedItem(self.width(), self.height())
        self.onThumbViewRequested()
        self.refresh()
        

    def updateView(self):

        if self._curr_view == self.thumb_view:
            self.onThumbViewRequested()

        elif self._curr_view == self.table_view:
            self.onTableViewRequested()


    def onParentSizeChangedItem(self, width, height):
        self.setWidth(width)
        self.setHeight(height)

        # self.search_bar.setWidth(width - 200)
        self.status_label.setPos(width - 100, 0)

        self.updateView()
        

    def refresh(self, sig_info=None):

        # self.hierarchy_nav.refresh()
        self.table_view.refresh()

        return

        if not self.edbc:
            self.status_label.setText("Error: Login required")
            return

        print('Querying for Media..')

        self.status_label.setText('Loading..')
        self.status_label.update()

        def _on_media_loaded(media_results):

            # pprint.pprint(media_results)
            # print('-------------')
            if not media_results:
                return

            # media_results = list(reversed(sorted(media_results, key=lambda x: x.get('created_at', ''))))

            count = 0
            total = len(media_results)

            for media_entry in media_results:
                count += 1
                media_entry['title'] = media_entry['code']
                if 'create_date' in media_entry:
                    media_entry['create_date'] = QtCore.QDateTime.fromSecsSinceEpoch(
                        media_entry['create_date']).toString().toStdString()

                if media_entry.get('image'):
                    self.thumbnail_client.requestImage(media_entry['image'], str(media_entry['id']))

            self.thumb_view.refresh(media_results)
            self.table_view.refresh(media_results)

            self.status_label.setText('Ready')

            self.update()



        # return_field_list = ['1uuid', 'title', 'name', 'comment', 'create_date', 'frame_count', 'thumb_frame_data', 'size', 'rfs_uuid']
        
        print('Media search filters:')
        pprint.pprint(self.searchFilters)


        # def _on_projects_ready(result):
        #     print('PROJECTS READY')
        #     self.status_label.setText('')
        #     print(result)

        # self.edbc.sg_find(_on_projects_ready, 'Project', [], ['code']).run()

        sequence_entity = {'type': 'Sequence', 'id': 11398}

        self.searchFilters = [
            ['project', 'is', project_entity],
            ['sg_sequence', 'is', sequence_entity],
        ]

        print(self.searchFilters)

        seq_filter = MediaFilter(self, 'Sequence', 'is', 'ID: 11398')
        self.filters.addItem(seq_filter, 0)
        self.filters.addSpacer(5)
        self._filters.append(seq_filter)

        # self.table_view.grid_view.searchReturnFields

        self.edbc.sg_find(_on_media_loaded,
            'Shot', self.searchFilters, ['id', 'code', 'image', 'created_at', 'sg_frame_range', 'sg_assignees']
        ).run()



    def get_selection(self):
        return self.thumb_view.get_selection()


    def onThumbRequestReady(self, data):
        self.thumb_view.onThumbImageReady(data['run_id'], data['image'])
        self.table_view.onThumbImageReady(data['run_id'], data['image'])


    def onThumbViewRequested(self, md=None):

        self._curr_view = self.thumb_view

        self.table_view.hideItem()
        self.table_view.setItemWidth(0)
        self.table_view.setItemHeight(0)

        self.thumb_view.showItem()
        self.thumb_view.onParentSizeChangedItem(self.width(), self.height())


    def onTableViewRequested(self, md=None):

        self._curr_view = self.table_view

        self.thumb_view.hideItem()
        self.thumb_view.setItemWidth(0)
        self.thumb_view.setItemHeight(0)


        self.table_view.showItem()
        self.table_view.onParentSizeChangedItem(self.width(), self.height())


    def onNavStatus(self, md):
        self.status_label.setText('Loading {}..'.format(md['name']))


    def onNavItemSelected(self, md):
        print('nav item selected')
        print(md)
        self.view_entity = md[1]


    def onNavItemsReady(self, md):

        self.status_label.setText('Loading..')
        self.status_label.update()

        entityResultList = md['data']
        for media_entry in entityResultList:
            # count += 1
            media_entry['title'] = media_entry['code']
            if 'create_date' in media_entry:
                media_entry['create_date'] = QtCore.QDateTime.fromSecsSinceEpoch(
                    media_entry['create_date']).toString().toStdString()

            if media_entry.get('image'):
                self.thumbnail_client.requestImage(media_entry['image'], str(media_entry['id']))


        self.thumb_view.refresh(entityResultList)
        self.table_view.refresh(entityResultList)

        self.status_label.setText('')

        self.view_entity = md['type']
        self.table_view.entity_type = self.view_entity
        # self.view_entity = {'type': md['type'], 'id': md.get('id')}

        self.update()


    def parseSearchStr(self, searchStr):

        searchEntries = []

        currEntry = {}
        in_key = True
        in_val = False


        numChars = len(searchStr)
        pos = 0
        while pos < numChars:

            char = searchStr[pos]
            if char in [':', '=']:

                pos += 1
                val = ""
                char = searchStr[pos]

                while char == ' ': # eat whitespace
                    pos += 1
                    char = searchStr[pos]
                    
                if char in ['"', "'"]:
                    pos += 1

                    tchar = searchStr[pos]
                    while tchar not in ['"', "'"] and pos < numChars:
                        val += tchar
                        pos += 1
                        tchar = searchStr[pos]

                else:
                    while char != ' ' and pos < numChars:
                        val += char
                        pos += 1
                        char = searchStr[pos]



    def onSearchRequested(self, searchInfo):
        textEntered = searchInfo['text']

        print(textEntered)
        self.search_client.requestPOST('http://vmpostweb-dev-01:9003/search', {'search_str': textEntered}, "")


    def onSearchRequestReady(self, response):
        print('SEARCH REQUEST DONE')
        print(response)
        if not response:
            return

        if 'result' not in response:
            return
        
        if not response['result']:
            return

        result = json.loads(response['result'])
        if 'status' not in result:
            print('invalid response: {}'.format(response))
            return
        
        if result['status'] != 'OK':
            print('error running query')
            print(result)
            return

        # print(result)


        searchResultList = result['result']
        mediaResultList = []
        errorList = []
        for searchEntry in searchResultList:
            # count += 1

            if 'media_info' not in searchEntry:
                print('No media info, skipping {}'.format(searchEntry))
                continue

            # if 'video_tracks' not in searchEntry['media_info']:
            #     print('No video tracks, skipping {}'.format(searchEntry))
            #     continue

            try:
                pprint.pprint(searchEntry)
                print('')
                frameCount = searchEntry['media_info']['rate'] * searchEntry['media_info']['duration']

                mediaEntry = {
                    'title': searchEntry['name'],
                    'code': searchEntry['name'],
                    'description': searchEntry['description'],
                    'sg_path_to_frames': searchEntry['path'],
                    'frame_count': frameCount,
                    'id': searchEntry['uid'],
                    'type': 'Asset'
                }
                
                print(mediaEntry)
                print('')

                mediaResultList.append(mediaEntry)

            except:
                print('Error processing {}'.format(searchEntry))
                print(traceback.format_exc())
                errorList.append(searchEntry)


            # if 'create_date' in media_entry:
            #     media_entry['create_date'] = QtCore.QDateTime.fromSecsSinceEpoch(
            #         media_entry['create_date']).toString().toStdString()

            # if media_entry.get('image'):
            #     self.thumbnail_client.requestImage(media_entry['image'], str(media_entry['id']))


        self.thumb_view.refresh(mediaResultList)
        self.table_view.refresh(mediaResultList)

        self.status_label.setText('')

        self.view_entity = 'Version'
        self.table_view.entity_type = self.view_entity
        # self.view_entity = {'type': md['type'], 'id': md.get('id')}

        self.update()



    def onLoadRequested(self, md):
        print('LOAD')
        print(self.view_entity)
        
        # with open('/tmp/seq_info.json') as fh:
        #     result = json.load(fh)
        #     media_list = []
        #     # pprint.pprint(result)
        #     self.status_label.setText('Done')

        #     for entry in result[1:20]:
        #         media_list.append({
        #             'media.name': entry['code'],
        #             'media.frame_count': entry.get('frame_count', 24),
        #             'media.video': entry['sg_path_to_frames'],
        #             'media.audio': entry['sg_path_to_frames']
        #         })

        #     # revlens.cmds.load_media_list(media_list, 0, 1)
        #     revlens.CNTRL.getMediaManager().loadMediaList(media_list)


        if self.view_entity['type'] == 'Sequence':
            print('LOADING SEQUENCE')

