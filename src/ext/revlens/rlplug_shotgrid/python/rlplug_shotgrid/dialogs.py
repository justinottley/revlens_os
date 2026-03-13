#
# Copyright 2014-2024 Justin Ottley
#
# Licensed under the terms set forth in the LICENSE.txt file
#

from rlp import QtGui
from rlp.Qt import QStr

import rlp.util as RlpUtil
import rlp.gui as RlpGui

import revlens
import revlens.gui as RevlensGui


class ShotgridLoadDialog(RlpGui.GUI_ItemBase):

    _INSTANCE = None

    def __init__(self, floating_pane, media_title, media_info, thumb_path):
        RlpGui.GUI_ItemBase.__init__(self, floating_pane.body())

        self._fpane = floating_pane
        self._fpane.setPersistent(True)

        self.title = RlpGui.GUI_Label(self, media_title)
        self.thumb = RlpGui.GUI_IconButton(thumb_path, self, 120, 4)
        self.thumb.setWidth(self._fpane.width())
        self.thumb.setHoverColour(self.thumb.outlineColour())

        self.loader = RevlensGui.GUI_ItemLoader(self)
        self.loader.loadComplete.connect(self.onLoadComplete)
        self.loader.onParentSizeChanged(
            floating_pane.width(),
            floating_pane.height()
        )

        self.layout = RlpGui.GUI_VLayout(self)
        self.layout.addSpacer(10)
        self.layout.addItem(self.title, 5)
        self.layout.addSpacer(5)
        self.layout.addItem(self.thumb, 65)
        self.layout.addSpacer(5)
        self.layout.addItem(self.loader, 0)
        self.layout.setWidth(self._fpane.width())
        self.layout.setHeight(self._fpane.height())

        self.loader.onParentSizeChanged(
            floating_pane.width(),
            floating_pane.height()
        )

        self._fpane.setText('Load Media')
        self._fpane.setTextBold(True)

        self.setup(media_title, media_info, thumb_path)


    @classmethod
    def create(cls, media_title, media_info, thumb_path):
        if cls._INSTANCE:
            diag = cls._INSTANCE
            diag.setup(media_title, media_info, thumb_path)
            diag._fpane.setVisible(True)
            return diag

        fpane = revlens.GUI.mainView().createFloatingPane(350, 400, False)
        diag = cls(fpane, media_title, media_info, thumb_path)
        diag._INSTANCE = diag

        return diag


    def setup(self, media_title, media_info, thumb_path):

        pdof = RlpUtil.APPGLOBALS.value('prod_data')
        cbMediaFormatter = pdof.getShowData().formatter('code')
        mediaName, mediaMd = cbMediaFormatter(media_info)

        self.title.setText(media_title)
        self.thumb.setIconPath(thumb_path)
        load_info = {
            'media.video': media_info['sg_path_to_frames'],
            'media.frame_count': media_info['frame_count'],
            'media.name': mediaName,
            'media.metadata': mediaMd
        }
        self.loader.setLoadInfo(load_info)


    def onParentSizeChangedItem(self, width, height):
        self.setWidth(width)
        self.setHeight(height)


    def onLoadComplete(self):
        self._fpane.requestClose()



class ShotgridSequenceLoadDialog(RlpGui.GUI_ItemBase):

    _INSTANCE = None

    def __init__(self, floating_pane, seq_info):
        RlpGui.GUI_ItemBase.__init__(self, floating_pane.body())

        self._fpane = floating_pane
        self._fpane.setTextBold(True)

        self.seq_info = seq_info
        self.pref_dept_list = []
        self.pref_fallbacks = True
        self.pref_order = 'most_recent'

        self.title = RlpGui.GUI_Label(self, seq_info['code'])

        self.load_text = RlpGui.GUI_Label(self, 'Dept:')
        self.load_text.setWidth(110)

        self.load_selector = RlpGui.GUI_IconButton('', self, 20, 4)
        self.load_selector.setText('Most Recent')
        self.load_selector.setOutlined(True)
        self.load_selector.setSvgIconPath(':feather/chevron-down.svg', 20, True)

        dept_menu = self.load_selector.menu()
        dept_menu.menuItemSelected.connect(self.on_dept_changed)

        dept_menu.addItem('Most Recent', {'dept_list': []}, False)
        dept_menu.addItem('Lighting', {'dept_list': ['lighting', 'animation', 'layout', 'editorial']}, False)
        dept_menu.addItem('Animation', {'dept_list': ['animation', 'layout', 'editorial']}, False)
        dept_menu.addItem('Editorial', {'dept_list': ['editorial']}, False)

        self.load_layout = RlpGui.GUI_HLayout(self)
        self.load_layout.addItem(self.load_text, 4)
        self.load_layout.addItem(self.load_selector, 0)

        self.fallback_text = RlpGui.GUI_Label(self, 'Use Fallbacks:')
        self.fallback_text.setWidth(110)

        self.fallback_btn = RlpGui.GUI_RadioButton(self, '', 16, 0)
        self.fallback_btn.setToggled(True)
        self.fallback_btn.buttonPressed.connect(self.on_fallback_toggled)

        self.fallback_layout = RlpGui.GUI_HLayout(self)
        self.fallback_layout.addItem(self.fallback_text, 0)
        self.fallback_layout.addItem(self.fallback_btn, 3)

        self.order_text = RlpGui.GUI_Label(self, 'Order By:')
        self.order_text.setWidth(110)

        self.order_selector = RlpGui.GUI_IconButton('', self, 20, 4)
        self.order_selector.setText('Most Recent')
        self.order_selector.setOutlined(True)
        self.order_selector.setSvgIconPath(':feather/chevron-down.svg', 20, True)
        # oi = self.order_selector.svgIcon()
        # oi.setFgColour(QtGui.QColor(140, 140, 140))
        # oi.setBgColour(QtGui.QColor(140, 140, 140))
        # oi.setPos(oi.x(), 4)

        order_menu = self.order_selector.menu()
        order_menu.menuItemSelected.connect(self.on_order_changed)
        order_menu.addItem('Most Recent', {'order':'most_recent'}, False)
        order_menu.addItem('Final', {'order': 'final'}, False)

        self.order_layout = RlpGui.GUI_HLayout(self)
        self.order_layout.addItem(self.order_text, 4)
        self.order_layout.addItem(self.order_selector, 0)

        self.load_button = RlpGui.GUI_IconButton('', self, 20, 4)
        self.load_button.setOutlined(True)
        self.load_button.setText('Load')
        self.load_button.buttonPressed.connect(self.on_load_requested)

        self.layout = RlpGui.GUI_VLayout(self)
        self.layout.addItem(self.title, 0)
        self.layout.addSpacer(10)
        self.layout.addItem(self.load_layout, 0)
        self.layout.addItem(self.fallback_layout, 0)
        self.layout.addSpacer(3)
        self.layout.addItem(self.order_layout, 0)
        self.layout.addSpacer(10)
        self.layout.addItem(self.load_button, 40)

        self.setup(seq_info)

        self.setWidth(self._fpane.width())
        self.setHeight(self._fpane.height())


    @classmethod
    def create(cls, seq_info):
        if cls._INSTANCE:
            diag = cls._INSTANCE
            diag.setup(seq_info)
            return diag

        fpane = revlens.GUI.mainView().createFloatingPane(350, 260, False)
        diag = cls(fpane, seq_info)
        diag._INSTANCE = diag

        return diag

    def setup(self, seq_info):
        self.seq_info = seq_info
        self.title.setText(self.seq_info['code'])
        self._fpane.setText('Load {}'.format(self.seq_info['type']))


    def on_dept_changed(self, md):
        self.load_selector.setText(md['text'])
        self.load_selector.update()
        
        self.pref_dept_list = md['data']['dept_list']


    def on_fallback_toggled(self, md):
        self.fallback_btn.setToggled(not self.fallback_btn.isToggled())
        self.pref_fallbacks = self.fallback_btn.isToggled()


    def on_order_changed(self, md):
        self.order_selector.setText(md['text'])
        self.order_selector.update()

        self.pref_order = md['data']['order']


    def on_load_requested(self, md=None):

        filters = []
        pref_dept_list = self.pref_dept_list
        if not self.pref_fallbacks:
            pref_dept_list = pref_dept_list[0:1]

        if self.pref_dept_list:
            filters = [['sg_department', 'in', pref_dept_list]]

        title =self.seq_info['code']
        if self.pref_dept_list:
            title += ' {}'.format(self.pref_dept_list[0].capitalize())

        import rlplug_shotgrid.cmds
        rlplug_shotgrid.cmds.request_load(
            self.seq_info['type'],
            self.seq_info,
            title=title,
            filters=filters,
            pref_dept_list=pref_dept_list,
            pref_order=self.pref_order
        )

        self._fpane.requestClose()