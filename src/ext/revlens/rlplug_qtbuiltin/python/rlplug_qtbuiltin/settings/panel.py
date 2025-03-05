'''
Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
'''

import os
import json
import logging
import tempfile
import traceback

from rlp import QtGui

import revlens
import revlens.gui


from .cntrl import SettingsController, ONE_MILLION


class SettingsOptionsPanel(QtGui.QDialog):

    def __init__(self):
        QtGui.QDialog.__init__(self, revlens.VIEW)

        self.LOG = logging.getLogger('rlp.{ns}.{cls}'.format(
            ns=__name__, cls=self.__class__.__name__))
        
        self.scntrl = SettingsController.instance()

        main_layout = QtGui.QVBoxLayout()
        
        app_globals = rlp_util.APPGLOBALS.globals()

        curr_size = int(self.scntrl.cache_lookahead.maxSize() / float(ONE_MILLION))


        # Video Lookahead Cache Size
        #
        self.size_widget = QtGui.QSpinBox(self)
        
        self.size_widget.setMinimum(10)
        self.size_widget.setMaximum(100000) # No max really
        self.size_widget.setSingleStep(10)
        self.size_widget.setValue(curr_size)
        
        self.size_widget.valueChanged.connect(self.on_vlcache_size_changed)
        cache_size_val = self.scntrl.settings_lookahead.get('cache_size')
        if cache_size_val:

            self.LOG.info('loading setting: video lookahead cache size: {}'.format(cache_size_val))
            self.size_widget.setValue(cache_size_val)

        size_layout = QtGui.QHBoxLayout()
        size_layout.addWidget(QtGui.QLabel('Size:'))
        size_layout.addWidget(self.size_widget)
        size_layout.addWidget(QtGui.QLabel('MB'))
        
        # Backward frame count
        #
        self.backward_framecount_widget = QtGui.QSpinBox(self)
        self.backward_framecount_widget.setMinimum(0)
        self.backward_framecount_widget.setMaximum(1000)
        self.backward_framecount_widget.setValue(self.scntrl.cache_lookahead.maxBackwardFrameCount())
        self.backward_framecount_widget.valueChanged.connect(
            self.on_vlcache_backward_framecount_changed)
        
        backward_frame_count_val = self.scntrl.settings_lookahead.get('backward_frame_count')
        if backward_frame_count_val:
            self.backward_framecount_widget.setValue(backward_frame_count_val)

        back_fc_layout = QtGui.QHBoxLayout()
        back_fc_layout.addWidget(QtGui.QLabel('Backward Frame Count:'))
        back_fc_layout.addWidget(self.backward_framecount_widget)
        back_fc_layout.addWidget(QtGui.QLabel('Frames'))
        
        
        
        # Video Lookahead Cache Group
        #
        vlcache_groupbox = QtGui.QGroupBox("Video - Lookahead Cache", self)
        vlcache_layout = QtGui.QVBoxLayout()
        vlcache_groupbox.setLayout(vlcache_layout)
        
        vlcache_layout.addLayout(size_layout)
        vlcache_layout.addLayout(back_fc_layout)
        
        # label = QtGui.QLabel('Size: {val} MB'.format(val=curr_size / 1000000.0))
        
        main_layout.addWidget(vlcache_groupbox)

        # Sync
        #
        sync_groupbox = QtGui.QGroupBox("Sync", self)
        sync_groupbox_layout = QtGui.QVBoxLayout()
        sync_groupbox.setLayout(sync_groupbox_layout)

        # sync_annotations_layout = QtGui.QHBoxLayout()
        # sync_annotations_layout.addWidget(QtGui.QLabel("Sync A"))
        self.sync_annotations_widget = QtGui.QCheckBox("Sync Annotations")
        self.sync_annotations_widget.setChecked(False)
        self.sync_annotations_widget.stateChanged.connect(
            self.on_sync_settings_changed
        )

        sync_groupbox_layout.addWidget(self.sync_annotations_widget)

        self.sync_media_widget = QtGui.QCheckBox('Sync Media Data (restart required)')
        self.sync_media_widget.setChecked(True)
        self.sync_media_widget.stateChanged.connect(
            self.on_sync_settings_changed
        )

        sync_groupbox_layout.addWidget(self.sync_media_widget)
        
        settings_sync = self.scntrl.get_settings(self.scntrl.SETTINGS_SYNC)
        if settings_sync:
            do_sync_annotations = settings_sync.get('annotations', True)
            self.sync_annotations_widget.setChecked(do_sync_annotations)

            do_sync_media_data = settings_sync.get('media_data', True)
            self.sync_media_widget.setChecked(do_sync_media_data)

        

        main_layout.addWidget(sync_groupbox)


        # Rooms - Data Sync Server
        #
        # self.room_server_widget = QtGui.QLineEdit(self)
        # self.room_server_widget.textChanged.connect(self.on_services_changed)
        # 
        # site_url = ':'.join(app_globals['site_url'].split(':')[0:-1])
        # rooms_server_url = '{}:7401'.format(site_url)
        # 
        # self.room_server_widget.setText(rooms_server_url)
        # 
        # roomserver_layout = QtGui.QHBoxLayout()
        # roomserver_layout.addWidget(QtGui.QLabel('Room Server: '))
        # roomserver_layout.addWidget(self.room_server_widget)
        # 
        # 
        # main_layout.addLayout(roomserver_layout)
        # 
        # Web Server
        #
        # self.web_server_widget = QtGui.QLineEdit(self)
        # self.web_server_widget.textChanged.connect(self.on_services_changed)
        # 
        # webserver_layout = QtGui.QHBoxLayout()
        # webserver_layout.addWidget(QtGui.QLabel('Web Server: '))
        # webserver_layout.addWidget(self.web_server_widget)
        # 
        # main_layout.addLayout(webserver_layout)

        # Remote File Server
        # self.remote_file_server_widget = QtGui.QLineEdit(self)
        # self.remote_file_server_widget.textChanged.connect(self.on_services_changed)


        # fileserver_layout = QtGui.QHBoxLayout()
        # fileserver_layout.addWidget(QtGui.QLabel('Remote File Server:'))
        # fileserver_layout.addWidget(self.remote_file_server_widget)

        # main_layout.addLayout(fileserver_layout)

        self.clear_cache_button = QtGui.QPushButton('Clear Disk Cache')
        self.clear_cache_button.clicked.connect(self.on_clear_cache_clicked)

        main_layout.addWidget(self.clear_cache_button)

        self.setLayout(main_layout)
        

    @classmethod
    def create(cls):
        return SettingsOptionsPanel()

    def on_vlcache_size_changed(self, size):
        
        size_in_bytes = size * ONE_MILLION
        
        self.LOG.debug('setting lookahead cache max size: {ms} MB - {msb} bytes'.format(
            ms=size, msb=size_in_bytes))

        self.cache_lookahead.setMaxSize(size_in_bytes)
        self.save_state_lookahead()

        import rlplug_qtbuiltin.cmds
        rlplug_qtbuiltin.cmds.display_message('Lookahead Cache Max Size: {ms} MB'.format(
            ms=size))


    def on_vlcache_backward_framecount_changed(self, frames):
        
        self.LOG.debug('setting lookahead backward frame count max: {fc} frames'.format(
            fc=frames))
        
        self.cache_lookahead.setMaxBackwardFrameCount(frames)
        self.save_state_lookahead()

        import rlplug_qtbuiltin.cmds
        rlplug_qtbuiltin.cmds.display_message('Lookahead Cache Max Backward Frame Count: {f} frames'.format(
            f=frames))


    def on_sync_settings_changed(self, switched):

        ann_state = self.sync_annotations_widget.isChecked()
        media_data_state = self.sync_media_widget.isChecked()

        settings_sync_path = os.path.join(self.scntrl.state_root_dir, self.scntrl.SETTINGS_SYNC)
        sync_data = {
            'annotations': ann_state,
            'media_data': media_data_state
        }

        with open(settings_sync_path, 'w') as wfh:
            wfh.write(json.dumps(sync_data, indent=2))

        self.LOG.debug('wrote {}'.format(settings_sync_path))


    def on_clear_cache_clicked(self):

        disk_cache_dir = os.path.join(
            tempfile.gettempdir(),
            'revlens',
            'file_cache',
            'production'
        )

        if os.path.isdir(disk_cache_dir):
            for root, dirs, files in os.walk(disk_cache_dir, topdown=False):
                for file_entry in files:
                    file_path = os.path.join(root, file_entry)

                    self.LOG.info('removing {}'.format(file_path))
                    os.remove(file_path)


    def save_state_lookahead(self):

        cache_size = int(self.cache_lookahead.maxSize() / float(ONE_MILLION))
        backward_frame_count = self.cache_lookahead.maxBackwardFrameCount()

        vl_cache_state_path = os.path.join(self.state_root_dir, self.SETTING_LOOKAHEAD_FILE)
        with open(vl_cache_state_path, 'w') as wfh:
            wfh.write(json.dumps({
                'backward_frame_count': backward_frame_count,
                'cache_size': cache_size
            }))

        self.LOG.info('Wrote {}'.format(vl_cache_state_path))
