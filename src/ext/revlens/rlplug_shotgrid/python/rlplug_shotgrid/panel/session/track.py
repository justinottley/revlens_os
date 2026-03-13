

from rlp import QtCore, QtGui

from revlens_prod.panel.session import media_data as prod_media_data

class ShotGridTrackMediaWidgetHandler(prod_media_data.TrackMediaWidgetHandler):

    def __init__(self, trackMediaWidget):
        prod_media_data.TrackMediaWidgetHandler.__init__(self, trackMediaWidget)


    def onGridModelDataChanged(self):
        pass

    def onGridCellWidgetEvent(self, evtName, evtInfo):
        print('{} {}'.format(evtName, evtInfo))

    def onContextMenuItemSelected(self, md):
        print(md)