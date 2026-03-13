
import pprint

from rlp import QtCore
import rlp.gui as RlpGui

from rlp.core.net.websocket import RlpClient

class BreadcrumbWidget(RlpGui.GUI_ItemBase):

    HEIGHT = 30

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.pattern = None

        self.hierarchyButtons = {}
        self.hselected = {} # selected hierarchy
        self.selectedHierarchyType = None

        self._seps = []

        self.navItemsReady = QtCore.PY_Signal(self)

        self.patternLayout = RlpGui.GUI_HLayout(self)

        # self.tpatternLayout = RlpGui.GUI_HLayout(self)
        # self.tpatternLayout.setBgColour(QtGui.QColor(60, 60, 60))
        # # self.tpatternLayout.setHeightOffset(4)
        # self.tpatternLayout.setWidthOffset(10)

        # self.tpatternLayout.addSpacer(10)
        # self.tpatternLayout.addItem(self.hierarchyButton, 2)
        # self.tpatternLayout.addSpacer(10)

       #  self.tpatternLayout.addItem(self.search_bar, 0)
        # self.tpatternLayout.addSpacer(10)
        # self.tpatternLayout.addItem(self.hierarchySep, 5)
        # self.tpatternLayout.addSpacer(5)
        # self.tpatternLayout.addItem(self.patternLayout, 0)

        self.setHeight(self.HEIGHT)
        self.setWidth(parent.width())


    @property
    def edbc(self):
        return RlpClient.instance()


    def setHierarchy(self, hierarchyInfo):

        self.pattern = hierarchyInfo

        # -----

        self.patternLayout.clear()
        self.patternLayout.addSpacer(15)

        self._seps.clear()
        self.hierarchyButtons.clear()

        idx = 1

        for entry in self.pattern:
            
            if 'selectable' in entry and not entry['selectable']:
                break

            # if self._endAtStep and entry['type'] == 'Step':
            #     break

            if idx > 1 and idx <= len(self.pattern):

                sep = RlpGui.GUI_Label(self, ' | ')
                self._seps.append(sep)
                self.patternLayout.addSpacer(3)
                self.patternLayout.addItem(sep, 0)


            entryButton = RlpGui.GUI_IconButton('', self, 20, 2)
            entryButton.setTextYOffset(-1)
            entryButton.setIconYOffset(-1)
            entryButton.setText('{}'.format(entry['type']))
            entryButton.setSvgIconPath(
                ':/feather/chevron-down.svg', 20, True)
            # entryButton.setSvgIconShowOnHover(True)
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


        self.patternLayout.onItemSizeChanged()
        self.patternLayout.update()
        # self.tpatternLayout.onItemSizeChanged()
        # self.tpatternLayout.update()
        self.setWidth(self.patternLayout.width() + 40)


    def reset(self):

        for entry in self.pattern:

            entryType = entry['type']
            if entryType not in self.hierarchyButtons:
                print('WARNING: no button for type {}'.format(entryType))
                continue

            hbutton = self.hierarchyButtons.get(entryType)
            hbutton.setText(entryType)


    def onItemSelected(self, md):

        et_name = md['data']['type']

        idx = 1
        for entry in self.pattern:
            if entry['type'] == et_name:
                break
            idx += 1

        self.hselected[idx] = md['data']
        self.hierarchyButtons[et_name].setText(md['text'])

        if len(self.pattern) == idx:
            self.endSelected.emit(self.hselected)


        else:

            next_et = self.pattern[idx]
            next_et_name = next_et['type']

            # if self._endAtStep and next_et_name == 'Step':
            #     endInfo = md['data'].copy()
            #     endInfo['pattern'] = self.pattern[-1]
            #     self.endSelected.emit(endInfo)
            #     return


            # print('Next ET: {}'.format(next_et))

            field_name = next_et['field_link']
            nextFilterValue = {'type': md['data']['type'], 'id': md['data']['id']}
            nextFilterLinkField = next_et.get('field_link_field')
            if nextFilterLinkField:
                nextFilterValue = md['data'][nextFilterLinkField]
                # nextFilterValueUuidVal = '{' + nextFilterValue['uuid'] + '}'
                # nextFilterValue['id'] = nextFilterValueUuidVal

            # print('Next Value: {}'.format(nextFilterValue))
            # raise Exception('stop')

    
            next_filter = [[field_name, 'is', nextFilterValue]]
            next_return_fields = ['name']
            if 'return_fields' in next_et:
                next_return_fields += next_et['return_fields']

            displayField = 'name'
            if 'display_field' in next_et:
                displayField = next_et['display_field']

            def _on_result(all_results):

                # print('got result for {} - displaying {}'.format(next_et_name, displayField))

                # TODO FIXME: DB / query level sort?
                # all_results = sorted(all_results, key=lambda x: x[displayField])

                # pprint.pprint(all_results)

                if next_et_name in self.hierarchyButtons:

                    hbutton = self.hierarchyButtons[next_et_name]
                    hbutton.setText(next_et_name)
                    hbutton.update()

                    hmenu = hbutton.menu()
                    hmenu.clear()

                    for entry in all_results: # reversed(all_results):
                        hmenu.addItem(
                            entry[displayField],
                            entry,
                            False
                        )
                    hmenu.updateScrolling()


                self.navItemsReady.emit({
                    'type': et_name,
                    'text': md['text'],
                    'id': md['data']['id'],
                    'data': all_results
                })


            if next_et_name in self.hierarchyButtons:
                self.hierarchyButtons[next_et_name].setText('[.....]')
                self.hierarchyButtons[next_et_name].update()


            # self.status.emit({'name': md['text']})


            iosg = self.edbc.ioclient('sg')
            if iosg:
                iosg.call(_on_result, 'cmds.find', next_et_name,
                    next_filter, next_return_fields).run()


    def selectItem(self, itemInfo):

        itemType = itemInfo['type']
        if itemType not in self.hierarchyButtons:
            print('ERROR: UNKNWN Hierarchy type: {}'.format(itemType))  
            return

        self.onItemSelected({
            'data': itemInfo,
            'text': itemInfo['code']
        })