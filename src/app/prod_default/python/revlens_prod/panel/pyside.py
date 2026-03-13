

_PANELS = []

from PySide6 import QtCore, QtWidgets

from rlp import QtCore as RlpQtCore
import rlp.gui as RlpGui
import revlens.cmds

from RlpGuiSBKPYMODULE import GUI_QWidgetWrapper

class TestThread(QtCore.QThread):

    def __init__(self):
        QtCore.QThread.__init__(self)


    def run(self):
        print('THREAD RUNNING!!')
        # requires QSG_RENDER_LOOP='basic'
        # self.panel.timer.start()
        # self.panel.testFunc()


class PySidePanel(RlpGui.GUI_ItemBase):

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.timer = RlpQtCore.PyTimer()
        self.timer.setInterval(10)
        self.timer.connectTimeout(self.pyCheck)
        self.timer.stop()

        self.qwrapper = GUI_QWidgetWrapper(self)
        self.qwrapper.setWidth(parent.width())
        self.qwrapper.setHeight(parent.height())

        self.w = QtWidgets.QWidget()

        self.lbl = QtWidgets.QLabel(self.w)
        self.lbl.setText("PySide6 Example Label")

        self.btn = QtWidgets.QPushButton("Toggle Play / Pause", self.w)
        self.btn.clicked.connect(self.onPlayPauseClicked)

        self.nwin = QtWidgets.QPushButton("New PySide Window", self.w)
        self.nwin.clicked.connect(self.onNewWinClicked)

        self.test = QtWidgets.QPushButton("Test Thread", self.w)
        self.test.clicked.connect(self.onTestThread)

        self.ta = QtWidgets.QTextEdit(self.w)

        self.l = QtWidgets.QVBoxLayout()
        self.l.addWidget(self.lbl)
        self.l.addWidget(self.btn)
        self.l.addWidget(self.nwin)
        self.l.addWidget(self.test)
        self.l.addWidget(self.ta)
        self.w.setLayout(self.l)

        self.qwrapper.setPySideWidget(self.w)

        self.windows = []
        self.objects = []

        self.onParentSizeChangedItem(parent.width(), parent.height())

    def pyCheck(self):
        RlpQtCore.QCoreApplication.pingPyInterp()
        self.timer.stop()

    def mousePressEventItem(self, event):
        self.qwrapper.forwardMouseEventToWidget(event)

    def mouseReleaseEventItem(self, event):
        self.qwrapper.forwardMouseEventToWidget(event)

    def mouseMoveEventItem(self, event):
        self.qwrapper.forwardMouseEventToWidget(event)

    def onPlayPauseClicked(self, isChecked):
        print('Toggle Play / Pause')
        revlens.cmds.toggle_play_pause()

    def onNewWinClicked(self, isChecked):
        print('open new window')
        pane = QtWidgets.QDialog()
        pane.resize(800, 600)

        te = QtWidgets.QTextEdit(pane)
        be = QtWidgets.QPushButton("Test", pane)
        l = QtWidgets.QVBoxLayout()
        l.addWidget(te)
        l.addWidget(be)

        pane.setLayout(l)
        pane.show()

        # need to stash the pane or else it will be garbage collected
        # and never show
        self.windows.append(pane)

    def onTestThread(self):

        t = TestThread()
        self.objects.append(t)
        t.start()
        # requires QSG_RENDER_LOOP='basic'
        # self.timer.start()

    def testFunc(self):
        print('test func running')

    def onParentSizeChangedItem(self, width, height):

        self.setWidth(width)
        self.setHeight(height)


def create(parent):
    panel = PySidePanel(parent)
    _PANELS.append(panel)
    return panel