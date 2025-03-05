
import os
import uuid
import traceback

from rlp.Qt import QStr, QVMap
from rlp import QtCore, QtGui

import rlp.util as RlpUtil
import rlp.gui as RlpGui

from rlp.gui.dialogs import InfoDialog

import revlens
import revlens.gui as RevlensGui


class ExportDialog(RlpGui.GUI_ItemBase):

    LABEL_WIDTH = 160
    _UI = []

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent.body())

        self._fpane = parent
        self._fpane.setText('Export Session')
        self._fpane.setTextBold(True)

        self.start_frame_label = RlpGui.GUI_Label(self, 'Start Frame:')
        self.start_frame_label.setWidth(self.LABEL_WIDTH)
        self.start_frame_input = RlpGui.GUI_TextEdit(self, 170, 60)
        self.start_frame_layout = RlpGui.GUI_HLayout(self)
        self.start_frame_layout.addItem(self.start_frame_label, 0)
        self.start_frame_layout.addItem(self.start_frame_input, 0)

        self.end_frame_label = RlpGui.GUI_Label(self, 'End Frame:')
        self.end_frame_label.setWidth(self.LABEL_WIDTH)
        self.end_frame_input = RlpGui.GUI_TextEdit(self, 170, 60)
        self.end_frame_layout = RlpGui.GUI_HLayout(self)
        self.end_frame_layout.addItem(self.end_frame_label, 0)
        self.end_frame_layout.addItem(self.end_frame_input, 0)

        self.width_label = RlpGui.GUI_Label(self, 'Output Width:')
        self.width_label.setWidth(self.LABEL_WIDTH)
        self.width_input = RlpGui.GUI_TextEdit(self, 170, 60)
        self.width_layout = RlpGui.GUI_HLayout(self)
        self.width_layout.addItem(self.width_label, 0)
        self.width_layout.addItem(self.width_input, 0)

        self.height_label = RlpGui.GUI_Label(self, 'Output Height:')
        self.height_label.setWidth(self.LABEL_WIDTH)
        self.height_input  = RlpGui.GUI_TextEdit(self, 170, 60)
        self.height_layout = RlpGui.GUI_HLayout(self)
        self.height_layout.addItem(self.height_label, 0)
        self.height_layout.addItem(self.height_input, 0)

        self.path_label = RlpGui.GUI_Label(self, 'Output Dir:')
        self.path_label.setWidth(self.LABEL_WIDTH)
        self.path_input = RlpGui.GUI_TextEdit(self, 170, 60)
        self.path_layout = RlpGui.GUI_HLayout(self)
        self.path_browse = RlpGui.GUI_SvgButton(':misc/open.svg', self, 30)
        self.path_browse.buttonPressed.connect(self.on_browse_requested)

        self.path_layout.addItem(self.path_label, 0)
        self.path_layout.addItem(self.path_input, 0)
        self.path_layout.addSpacer(15)
        self.path_layout.addItem(self.path_browse, 0)

        self.fp_path_label = RlpGui.GUI_Label(self, 'Filename Prefix:')
        self.fp_path_label.setWidth(self.LABEL_WIDTH)
        self.fp_path_input = RlpGui.GUI_TextEdit(self, 170, 60)
        self.fp_path_layout = RlpGui.GUI_HLayout(self)
        self.fp_path_layout.addItem(self.fp_path_label, 0)
        self.fp_path_layout.addItem(self.fp_path_input, 0)

        self.output_format_label = RlpGui.GUI_Label(self, 'Output Format:')
        self.output_format_label.setWidth(self.LABEL_WIDTH)

        self.output_format = RlpGui.GUI_IconButton('', self, 20, 4)
        self.output_format.setText('Format:')
        self.output_format.setOutlined(True)
        self.output_format.setSvgIconPath(':feather/lightgrey/chevron-down.svg', 20, True)
        icon = self.output_format.svgIcon()
        icon.setPos(icon.x(), 4)
        icon.setBgColour(QtGui.QColor(160, 160, 160))

        omenu = self.output_format.menu()
        omenu.addItem('png', {}, False)
        omenu.addItem('jpg', {}, False)
        omenu.addItem('mov', {}, False)
        omenu.addItem('mp4', {}, False)
        omenu.addItem('avi', {}, False)
        omenu.menuItemSelected.connect(self.on_output_format_selected)

        self.output_format_layout = RlpGui.GUI_HLayout(self)
        self.output_format_layout.addItem(self.output_format_label, 0)
        self.output_format_layout.addItem(self.output_format, 0)
    
        self.export_button = RlpGui.GUI_IconButton("", self, 20, 4)
        self.export_button.setText('Export')
        self.export_button.setOutlined(True)
        self.export_button.buttonPressed.connect(self.run_export)

        self.cancel_button = RlpGui.GUI_IconButton("", self, 20, 4)
        self.cancel_button.setText('Cancel')
        self.cancel_button.setOutlined(True)
        self.cancel_button.buttonPressed.connect(self.close)

        self.status_text = RlpGui.GUI_Label(self, '')

        self.button_layout = RlpGui.GUI_HLayout(self)
        self.button_layout.addSpacer(110)
        self.button_layout.addItem(self.export_button, 0)
        self.button_layout.addSpacer(30)
        self.button_layout.addItem(self.cancel_button, 0)

        self.layout = RlpGui.GUI_VLayout(self)
        self.layout.addSpacer(10)
        self.layout.addItem(self.start_frame_layout, 0)
        self.layout.addItem(self.end_frame_layout, 0)
        self.layout.addItem(self.width_layout, 0)
        self.layout.addItem(self.height_layout, 0)
        self.layout.addItem(self.path_layout, 0)
        self.layout.addItem(self.fp_path_layout, 0)
        
        self.layout.addItem(self.output_format_layout, 0)
        self.layout.addSpacer(10)
        self.layout.addItem(self.status_text, 100)
        self.layout.addSpacer(10)
        self.layout.addItem(self.button_layout, 0)

        self.init_values()

        self._dir_evt_uuid = str(uuid.uuid4())

        revlens.GUI.directoryChosen.connect(self.on_dir_chosen)

        self.setItemWidth(self._fpane.width())
        self.setItemHeight(self._fpane.height())


    def init_values(self):

        sess = revlens.CNTRL.session()

        self.start_frame_input.setText(str(sess.startFrame()))
        self.end_frame_input.setText(str(sess.frameCount()))

        self.fp_path_input.setText('export')

        node = sess.getNodeByFrame(1)
        if not node:
            return
        np = node.getProperties()

        try:

            frame_info = np['video.frame_info.one']
            self.width_input.setText(str(frame_info['width']))
            self.height_input.setText(str(frame_info['height']))

        except:
            print(traceback.format_exc())


    def on_output_format_selected(self, md):
        self.output_format.setText(md['text'])


    def on_dir_chosen(self, evt_uuid, sel_dir):

        if evt_uuid != self._dir_evt_uuid:
            return

        self.path_input.setText(sel_dir)
        self.path_input.update()
        self.path_input.textArea().update()


    def on_browse_requested(self, md):
        revlens.GUI.chooseDirectory(self._dir_evt_uuid)


    def close(self, md=None):
        self._fpane.requestClose()


    def run_export(self, md=None):

        import revlens.cmds

        start_frame = self.start_frame_input.text()
        end_frame = self.end_frame_input.text()
        path = self.path_input.text()
        width = self.width_input.text()
        height = self.height_input.text()
        frame_prefix = self.fp_path_input.text()
        output_format = self.output_format.text()

        if path == '':
            self.status_text.setText('Error: No Path!')
            return
            
        if output_format == 'Format:':
            self.status_text.setText('Error: No Output Format!')
            return

        if output_format in ['mov', 'mp4', 'avi']:
            frame_format = 'png'

        else:
            frame_format = output_format


        print('{} - {}'.format(start_frame, end_frame))
        print(path)
        print('{} x {}'.format(width, height))

        self.status_text.setText('Exporting..')

        print('RUN EXPORT')


        try:
            revlens.cmds.export(
                start_frame,
                end_frame,
                path,
                width,
                height,
                frame_format=frame_format,
                frame_prefix=frame_prefix,
                output_format=output_format
            )
        except:
            print(traceback.format_exc())

        self.close()

        InfoDialog.create(
            'Revlens',
            'Export Complete',
            icon='check-circle'
        )



    @classmethod
    def create(cls, width=400):

        pane = revlens.VIEW.createFloatingPane(width, 580, False)
        panel = cls(pane)

        panel._fpane.setItemWidth(width)
        cbtn = panel._fpane.closeButton()
        cbtn.setPos(panel._fpane.width() - cbtn.width() - 10, 6)

        cls._UI.append(panel)

        return panel



class AboutDialog(RlpGui.GUI_ItemBase):

    _UI = []

    LABEL_WIDTH = 150

    def __init__(self, fpane):
        RlpGui.GUI_ItemBase.__init__(self, fpane.body())

        self._fpane = fpane
        self._fpane.setText('About Revlens')
        self._fpane.setTextBold(True)

        ag = RlpUtil.APPGLOBALS
        build_time = ag.buildTime()
        build_version = ag.buildVersion()

        self.time_layout = RlpGui.GUI_HLayout(self)

        self.build_time_label = RlpGui.GUI_Label(self, 'Build Time:')
        self.build_time_label.setWidth(self.LABEL_WIDTH)

        self.build_time_value = RlpGui.GUI_Label(self, build_time.toString().toStdString())

        self.time_layout.addSpacer(10)
        self.time_layout.addItem(self.build_time_label, 0)
        self.time_layout.addItem(self.build_time_value, 0)

        self.version_layout = RlpGui.GUI_HLayout(self)

        self.build_version_label = RlpGui.GUI_Label(self, 'Build Version:')
        self.build_version_label.setWidth(self.LABEL_WIDTH)

        self.build_version_value =  RlpGui.GUI_Label(self, build_version)

        self.version_layout.addSpacer(10)
        self.version_layout.addItem(self.build_version_label, 0)
        self.version_layout.addItem(self.build_version_value, 0)

        self.layout = RlpGui.GUI_VLayout(self)
        self.layout.addSpacer(10)
        self.layout.addItem(self.time_layout, 0)
        self.layout.addItem(self.version_layout, 0)

        self.setItemWidth(self._fpane.width())
        self.setItemHeight(self._fpane.height())



    @classmethod
    def create(cls, width=400):

        pane = revlens.VIEW.createFloatingPane(width, 120, False)
        panel = cls(pane)

        panel._fpane.setItemWidth(width)
        cbtn = panel._fpane.closeButton()
        cbtn.setPos(panel._fpane.width() - cbtn.width() - 10, 6)

        cls._UI.append(panel)

        return panel


class SetupDialog(RlpGui.GUI_ItemBase):

    _UI = []

    def __init__(self, fpane):
        RlpGui.GUI_ItemBase.__init__(self, fpane.body())

        self._fpane = fpane

        self._fpane.setText('Setup / Update / Install')
        self._fpane.setTextBold(True)

        self.btn_create_desktop_shortcut = RlpGui.GUI_IconButton('', self, 20, 4)
        self.btn_create_desktop_shortcut.setText('Create Desktop Shortcut')
        self.btn_create_desktop_shortcut.setOutlined(True)
        self.btn_create_desktop_shortcut.buttonPressed.connect(self.create_desktop_shortcut)

        self.layout = RlpGui.GUI_VLayout(self)

        self.layout.addSpacer(40)
        self.layout.addItem(self.btn_create_desktop_shortcut, 10)

        self.setItemWidth(self._fpane.width())
        self.setItemHeight(self._fpane.height())


    def create_desktop_shortcut(self, md=None):

        if os.name != 'nt':
            return

        ag = RlpUtil.get_app_globals()

        import pprint
        pprint.pprint(ag)

        import rlp.core.util

        try:
            # check for disted top-level, version independent launcher
            top_launcher_root = os.path.dirname(
                os.path.dirname(os.path.dirname(os.path.dirname(ag['app.rootpath']))))
            top_launcher_path = os.path.join(top_launcher_root, 'revlens.bat')

            working_dir = None
            if not os.path.isfile(top_launcher_path):
                
                # dev..
                top_launcher_path = os.path.join(
                    ag['app.rootpath'],
                    'revlens', 'bin', 'revlens.bat'
                )
                working_dir = os.path.dirname(os.path.dirname(ag['app.rootpath']))


            icon_path = os.path.join(
                ag['app.rootpath'],
                'gui', 'static', 'icon_files', 'revlens_r.ico'
            )

            print(top_launcher_path)
            print(icon_path)

            result = rlp.core.util.create_desktop_shortcut(
                'Revlens',
                top_launcher_path,
                icon_path,
                working_dir
            )

            print('Got result: {}'.format(result))

            if result:
                InfoDialog.create(
                    'Revlens',
                    'Desktop Shortcut Created Successfully',
                    450,
                    icon='check-circle'
                )

        except:
            print(traceback.format_exc())


    @classmethod
    def create(cls, width=400):
        pane = revlens.VIEW.createFloatingPane(width, 220, False)
        panel = cls(pane)

        panel._fpane.setItemWidth(width)
        cbtn = panel._fpane.closeButton()
        cbtn.setPos(panel._fpane.width() - cbtn.width() - 10, 6)

        cls._UI.append(panel)

        return panel


class LoadDialog(RlpGui.GUI_ItemBase):

    def __init__(self, title, media_info, floating_pane):
        RlpGui.GUI_ItemBase.__init__(self, floating_pane.body())

        self._fpane = floating_pane

        self.setZValue(floating_pane.z())

        self._fpane = floating_pane
        self.media_info = media_info

        self.label = RlpGui.GUI_Label(self, title)
        self.loader = RevlensGui.GUI_ItemLoader(self)
        self.loader.setLoadPath(media_info)
        self.loader.loadComplete.connect(self.on_load_complete)

        self.layout = RlpGui.GUI_VLayout(self)
        self.layout.setItemWidth(self._fpane.width())
        self.layout.setItemHeight(self._fpane.height())

        self.layout.addSpacer(10)
        self.layout.addItem(self.label, 0)
        self.layout.addSpacer(20)
        self.layout.addItem(self.loader, 0)

        self._fpane.setText('Load Media')
        self._fpane.setTextBold(True)
        self._fpane.setItemHeight(280)

        self.loader.onParentSizeChanged(
            floating_pane.width(),
            floating_pane.height()
        )


    def on_load_complete(self):
        self._fpane.requestClose()

