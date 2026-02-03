
import json
import pprint

from rlp import QtCore, QtGui
import rlp.gui as RlpGui
import rlp.util as RlpUtil

from rlp.core.net.websocket import RlpClient


class HierarchyNavWidget(RlpGui.GUI_ItemBase):

    HEIGHT = 30

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.status = QtCore.PY_Signal(self)
        self.navItemsReady = QtCore.PY_Signal(self)
        self.endSelected = QtCore.PY_Signal(self)
        self.searchRequested = QtCore.PY_Signal(self)


        self.edbc = RlpClient.instance()

        self._endAtStep = False # TODO FIXME: REMOVE

        self.hierarchyButtons = {}
        self.hselected = {} # selected hierarchy
        self.selectedHierarchyType = None

        self._seps = []

        self.hierarchyButton = RlpGui.GUI_SvgButton(
            ":misc/compass.svg", self, 18)

        self.hierarchyButton.setToolTipText('Select Hierarchy')
        self.hierarchyButton.setAutoHideMenu(False)
        # self.hierarchySep = RlpGui.GUI_Label(self, '>')

        self.hierarchyButton.menu().menuItemSelected.connect(self.onHierarchySelected)
        self.hierarchyButton.connectToMenu()
        self.hierarchyButton.menu().onParentSizeChanged(parent.width(), parent.height())

        self.search_bar = RlpGui.GUI_TextEdit(self, 400, 25)
        # self.search_bar.setVisible(False)
        self.search_bar.setTempHintText("Search:")
        self.search_bar.scrollArea().setHScrollBarVisible(False)
        self.search_bar.enterPressed.connect(self.onSearchRequested)

        self.patternLayout = RlpGui.GUI_HLayout(self)

        self.tpatternLayout = RlpGui.GUI_HLayout(self)
        self.tpatternLayout.setBgColour(QtGui.QColor(60, 60, 60))
        # self.tpatternLayout.setHeightOffset(4)
        self.tpatternLayout.setWidthOffset(10)

        # self.tpatternLayout.addSpacer(10)
        self.tpatternLayout.addItem(self.hierarchyButton, 2)
        self.tpatternLayout.addSpacer(10)

        self.tpatternLayout.addItem(self.search_bar, 0)
        self.tpatternLayout.addSpacer(10)
        # self.tpatternLayout.addItem(self.hierarchySep, 5)
        # self.tpatternLayout.addSpacer(5)
        self.tpatternLayout.addItem(self.patternLayout, 0)

        self.setHeight(self.HEIGHT)
        self.setWidth(parent.width())

    
    @property
    def project_entity(self):
        ag = RlpUtil.get_app_globals()
        if 'project.info' in ag:
            return {'type': 'Project', 'id': ag['project.info']['id']}


    def setEndAtStep(self, eat):
        self._endAtStep = eat


    def initHierarchyMenu(self):

        if not self.edbc:
            return

        hlist_menu = self.hierarchyButton.menu()
        hlist_menu.clear()

        def _load(eresult):

            if not eresult:
                return

            for einfo in eresult:
                hlist_menu.addItem(
                    einfo['name'],
                    einfo,
                    False
                )

        seq_hierarchy = json.dumps([
            {
                'type': 'Sequence',
                'field_link': 'p3roject',
                'return_fields': ['code', 'image'],
                'filters': [
                    ['sg_status_list', 'is', 'ip'],
                    ['sg_sequence_type', 'not_in', ['Test', 'CrowdCycle']]],
                'name_field': 'code',
                'order': [{'field_name': 'code', 'direction': 'asc'}]
            },
            {
                'type': 'Shot',
                'field_link':'sg_sequence',
                'return_fields': ['code', 'image'],
                'filters':[

                ],
                'name_field': 'code',
                'display_field': 'code',
                'order': [{'field_name': 'code', 'direction': 'asc'}]
            },
            {
                'type': 'Version',
                'field_link': 'entity',
                'return_fields': ['code', 'image', 'description', 'user', 'sg_testid', 'sg_department', 'sg_path_to_frames', 'frame_count'],
                'name_field': 'code',
                'display_field': 'code',
                'selectable': False,
                'order': [{'field_name': 'updated_at', 'direction': 'desc'}]
            }
        ])
        revlist_hierarchy = json.dumps([
            {
                'type': 'Playlist',
                'field_link': 'project',
                'return_fields': ['code'],
                'filters': [['sg_status_list', 'is', 'act']],
                'name_field': 'code',
                'limit': 25,
                'order': [{'field_name': 'updated_at', 'direction': 'desc'}]
            }
        ])

        # {
        #     'type': 'Shot',
        #     'field_link': 'sg_sequence',
        #     'display_field': 'code',
        #     'return_fields': ['code', 'image', 'sg_frame_range']
        # }

        # self.edbc.call(_load, 'edbc.find', 'Hierarchy', [], ['name', 'pattern']).run()
        hlist = [
            {'name': 'Search..'},
            {'name': 'Sequence', 'pattern': seq_hierarchy},
            {'name': 'Reviewlist', 'pattern': revlist_hierarchy}
        ]
        _load(hlist)


    def onHierarchySelected(self, md):
        print(md)

        def _serviceCallback(result):
            print('RETURNED FROM SERVICE CALL')
            print(result)


        self.edbc.ioclient('sg').call(_serviceCallback, 'sg.get_reviewlists', ['RLP']).run()

        if md['text'].startswith('Search'): # hardcoded behavior
            self.patternLayout.clear()
            self._seps.clear()
            self.hierarchyButtons.clear()

            # self.initSearchBar()
            # self.patternLayout.addItem(self.search_bar, 0)
            self.search_bar.setWidth(400)
            self.search_bar.setVisible(True)
            return


        self.search_bar.setVisible(False)
        self.search_bar.setWidth(0)

        pattern = json.loads(md['data']['pattern'])
        #pprint.pprint(pattern)

        self.pattern = pattern
        self.selectedHierarchyType = md['text']

        self.patternLayout.clear()
        self.patternLayout.addSpacer(15)

        self._seps.clear()
        self.hierarchyButtons.clear()

        idx = 1
        for entry in pattern:
            
            if 'selectable' in entry and not entry['selectable']:
                break

            if self._endAtStep and entry['type'] == 'Step':
                break

            if idx > 1 and idx <= len(pattern):
                print('Adding Sep')
                sep = RlpGui.GUI_Label(self, '>')
                self._seps.append(sep)
                self.patternLayout.addSpacer(3)
                self.patternLayout.addItem(sep, 0)


            print('adding {}'.format(entry))

            entryButton = RlpGui.GUI_IconButton('', self, 20, 2)
            entryButton.setTextYOffset(-1)
            entryButton.setIconYOffset(-1)
            entryButton.setText('[{}]'.format(entry['type']))
            entryButton.setSvgIconPath(
                ":/feather/chevron-down.svg", 20, True)
            entryButton.setSvgIconShowOnHover(True)
            # entryButton.setShowMenuOnHover(True)
            entryButton.setMenuXOffset(30)
            ri = entryButton.svgIcon()
            ri.setPos(ri.x(), 2)


            entryButton.menu().menuItemSelected.connect(self.onItemSelected)

            entryButton.setMetadata('type', entry['type'])
            entryButton.setMetadata('return_fields', entry['return_fields'])
            entryButton.setMetadata('filters', entry.get('filters', []))

            self.hierarchyButtons[entry['type']] = entryButton

            self.patternLayout.addItem(entryButton, 0)

            idx += 1


        self.tpatternLayout.onItemSizeChanged()
        self.tpatternLayout.update()

        self.onHierarchyListed(pattern[0])


    def onHierarchyListed(self, md):
        print(md)
        if not self.project_entity:
            print('ERROR: Project not set, aborting')
            return

        et_name = md['type']
        
        hbutton = self.hierarchyButtons[et_name]
        hbutton_menu = hbutton.menu()
        hbutton_menu.clear()

        def _on_result(all_results):
            if not all_results:
                return

            # TODO FIXME: DB / query level sort?
            # all_results = sorted(all_results, key=lambda x: x[md['name_field']])

            for entry in all_results: # reversed(all_results):

                hbutton_menu.addItem(
                    entry[md['name_field']],
                    entry,
                    False
                )

            hbutton_menu.updateScrolling()

            print('populated: {}'.format(et_name))
            self.navItemsReady.emit({'type': et_name, 'data': all_results})


        self.status.emit({'name': et_name})

        filters = [['project', 'is', self.project_entity]]
        filters += md['filters']

        order_by = md.get('order')
        limit = md.get('limit', 0)

        # print('FILTERS TO FIND:')
        # print(filters)
        # print(order_by)
        # raise Exception('stop')

        self.edbc.sg_find(_on_result, et_name,
            filters,
            md['return_fields'],
            order=order_by,
            limit=limit
        ).run()


    def onItemSelected(self, md):
        print('ON ITEM SELEDCTED: \n{}'.format(pprint.pformat(md)))
        et_name = md['data']['type']

        idx = 1
        for entry in self.pattern:
            if entry['type'] == et_name:
                break
            idx += 1


        print('IDX: {}'.format(idx))

        self.hselected[idx] = md['data']
        self.hierarchyButtons[et_name].setText(md['text'])

        if len(self.pattern) == idx:
            self.endSelected.emit(self.hselected)


        else:

            next_et = self.pattern[idx]
            next_et_name = next_et['type']

            if self._endAtStep and next_et_name == 'Step':
                endInfo = md['data'].copy()
                endInfo['pattern'] = self.pattern[-1]
                self.endSelected.emit(endInfo)
                return


            print('Next ET: {}'.format(next_et))

            field_name = next_et['field_link']
            nextFilterValue = {'type': md['data']['type'], 'id': md['data']['id']}
            nextFilterLinkField = next_et.get('field_link_field')
            if nextFilterLinkField:
                nextFilterValue = md['data'][nextFilterLinkField]
                # nextFilterValueUuidVal = '{' + nextFilterValue['uuid'] + '}'
                # nextFilterValue['id'] = nextFilterValueUuidVal

            print('Next Value: {}'.format(nextFilterValue))
            # raise Exception('stop')

    
            next_filter = [[field_name, 'is', nextFilterValue]]
            next_return_fields = ['name']
            if 'return_fields' in next_et:
                next_return_fields += next_et['return_fields']

            displayField = 'name'
            if 'display_field' in next_et:
                displayField = next_et['display_field']

            def _on_result(all_results):

                print('got result for {} - displaying {}'.format(next_et_name, displayField))

                # TODO FIXME: DB / query level sort?
                # all_results = sorted(all_results, key=lambda x: x[displayField])

                # pprint.pprint(all_results)

                if next_et_name in self.hierarchyButtons:
                    hbutton = self.hierarchyButtons[next_et_name]
                    hmenu = hbutton.menu()
                    hmenu.clear()

                    for entry in all_results: # reversed(all_results):
                        hmenu.addItem(
                            entry[displayField],
                            entry,
                            False
                        )
                    hmenu.updateScrolling()

                self.navItemsReady.emit({'type': et_name, 'text': md['text'], 'id': md['data']['id'], 'data': all_results})


            print('RUNNING FIND')

            self.status.emit({'name': md['text']})

            self.edbc.sg_find(_on_result,
                next_et_name, next_filter, next_return_fields).run()


    def onSearchRequested(self, md):
        textEntered = md['item'].text()
        self.searchRequested.emit({'text':textEntered})

    def onParentSizeChangedItem(self, width, height):
        # self.setWidth(width)
        #self.setHeight(self.HEIGHT)
        pass


    def refresh(self):
        self.initHierarchyMenu()
