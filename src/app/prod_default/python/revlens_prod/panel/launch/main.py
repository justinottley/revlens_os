
import os
import platform
import tempfile

from rlp import QtCore, QtGui

import rlp.core as RlpCore
import rlp.gui as RlpGui
import rlp.util as RlpUtil

import rlp.core.path.util as rlp_path_util


from . import util as launch_util

_PANELS = []

class ServiceAppWidget(RlpGui.GUI_ItemBase):

    APP_WIDGET_HEIGHT = 50

    def __init__(self, parent, name, command, metadata):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.metadata = metadata
        self.command = command

        self.proc = RlpCore.PROC_Process()
        self.proc.finished.connect(self.onProcessFinished)

        self._bgCol = QtGui.QColor(60, 60, 60)
        self._fgCol = QtGui.QColor(80, 80, 80)

        self._col = self._fgCol
        self.name_label = RlpGui.GUI_Label(self, name)
        self.name_label.setPos(5, 15)
        nf = self.name_label.font()
        nf.setBold(True)
        nf.setPixelSize(nf.pixelSize() + 3)
        self.name_label.setFont(nf)

        self.run_button = RlpGui.GUI_IconButton("", self, 20, 5)
        self.run_button.setText("Start")
        self.run_button.setOutlined(True)
        self.run_button.buttonPressed.connect(self.on_run_toggled)

        self.setup_command()


    def setup_command(self):
        self.proc.setCommand(self.command)

    def on_run_toggled(self, md):

        print(self.proc.isRunning())

        if self.proc.isRunning():
            self.proc.stop()
            self.run_button.setText("Start")

        else:

            self.proc.start()
            self.run_button.setText("Stop")


    def onProcessFinished(self):
        print('process finished')


    def onParentSizeChangedItem(self, width, height):

        self.setWidth(width - 40)
        self.setHeight(self.APP_WIDGET_HEIGHT)

        self.name_label.setWidth(width - 20)
        self.run_button.setPos(width - 100, 10)


    def boundingRect(self):
        return QtCore.QRectF(0, 0, self.width(), self.height())

    def paintItem(self, painter):

        p = QtGui.QPen()
        p.setColor(QtGui.QColor(10, 10, 10))

        b = QtGui.QBrush(self._col)

        painter.setPen(p)
        painter.setBrush(b)

        br = self.boundingRect()

        painter.drawRect(br)


class DesktopStreamWidget(ServiceAppWidget):

    def __init__(self, parent, name, command, metadata):
        ServiceAppWidget.__init__(self, parent, name, command, metadata)


    def setup_command(self):

        app_globals = RlpUtil.APPGLOBALS.globals()
        self.proc.setKillTag('"rtsp_transport tcp"')
        cmd_exec = rlp_path_util.which('rlp_start_desktop_stream')

        cmd = '''{} --screen_idx {} --screen_name '{}' --resolution '{}' --position '{}' --server '{}' '''.format(
            cmd_exec,
            self.metadata['screen_idx'],
            self.metadata['screen_name'],
            self.metadata['resolution'],
            self.metadata['position'],
            app_globals.get('edb.site_hostname', 'postwebstage')
        )
        print(cmd)

        # cmd = 'sleep 100000'
        sh_text = '#!/bin/bash\n\n'
        sh_text += cmd

        temp_fd, temp_filename = tempfile.mkstemp(prefix='desktop_stream_', suffix='.sh')

        os.write(temp_fd, sh_text.encode('utf-8'))
        os.close(temp_fd)
        os.chmod(temp_filename, 0o777)

        print(temp_filename)
        self.proc.setCommand(temp_filename)


class LaunchPanel(RlpGui.GUI_ItemBase):

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.layout = RlpGui.GUI_VLayout(self)

        self.layout.addSpacer(20)

        self.apps = {}

        self.add_service('Mobile Data', 'rlp_start_frame_server.sh')

        for screenInfo in launch_util.get_screen_info():
            screenLabel = 'Desktop Stream: {}'.format(screenInfo['screen_name'])
            self.add_service(screenLabel, 'rlp_start_desktop_stream', screenInfo, DesktopStreamWidget)


        self.onParentSizeChangedItem(parent.width(), parent.height())


    def add_service(self, name, command, mdata=None, cls=ServiceAppWidget):

        widget = cls(self, name, command, mdata)
        self.layout.addItem(widget, 10)
        self.layout.addSpacer(10)

        self.apps[name] = widget


    def onParentSizeChangedItem(self, width, height):
        self.setWidth(width)
        self.setHeight(height)


def create(parent):
    global _PANELS
    result = LaunchPanel(parent)
    _PANELS.append(result)
    return result