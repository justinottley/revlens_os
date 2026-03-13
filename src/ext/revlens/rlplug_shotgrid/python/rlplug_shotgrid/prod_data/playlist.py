

from rlp import QtCore

import rlp.core as RlpCore
from rlp.core.net.websocket import RlpClient

import rlp.util as RlpUtil


class ShotGridPlaylistDataSource(QtCore.QObject):

    PATH_ROOTS = [
        'Animation',
        'Braintrust',
        'Bloopers',
        'Characters',
        'Crowds',
        'Editorial',
        'Effects',
        'Environments',
        'Env Assets',
        'Layout',
        'LFIN',
        'Lighting',
        'Marketing',
        'Misc',
        'Stereo',
        'Story',
        'Sweatbox',
        'Technical Animation',
        'Testing',
        'Training',
        'Visdev'
    ]

    def __init__(self):
        QtCore.QObject.__init__(self)

        self._project_entity = None # may not be available at init time yet


    @property
    def edbc(self):
        return RlpClient.instance()


    @property
    def project_entity(self):
        if self._project_entity is not None:
            return self._project_entity

        if RlpUtil.APPGLOBALS.contains('project.info'):
            projectInfo = RlpUtil.APPGLOBALS.value('project.info')
            self._project_entity = {
                'type': 'Project',
                'id': projectInfo['id'],
                'code': projectInfo['code']
            }

        return self._project_entity

    def init_static(self, callback, force=False):

        if self.edbc is None:
            return


        root = RlpCore.CoreDs_ModelDataItem.makeItem()
        for pathEntry in self.PATH_ROOTS:

            pi = RlpCore.CoreDs_ModelDataItem.makeItem()
            pi.setPopulatePyCallback(self.populatePath)

            pi.insert('name', pathEntry)

            ci = RlpCore.CoreDs_ModelDataItem.makeItem()
            pi.insertItem('__children__', ci)

            root.append(pi)

        callback(root)


    def init(self, callback, force=False):

        def _playlistsReady(result):

            root = RlpCore.CoreDs_ModelDataItem.makeItem()
            for entry in result:
                pi = RlpCore.CoreDs_ModelDataItem.makeItem()
                pi.insert('name', entry['code'])
                pi.insert('id', entry['id'])

                root.append(pi)

            callback(root)


        self.edbc.ioclient('sg').call(_playlistsReady,
            'cmds.find',
            'Playlist',
            [
                ['project', 'is', self.project_entity]
            ],
            ['code', 'id'],
        ).run()

    
    def populatePath(self, item):

        data = item.values()

        pathStart = '/{}/'.format(data['path'])
        filters = [
            ['project', 'is', self.project_entity],
            ['sg_status_list', 'is', 'act'],
            ['sg_review_path', 'not_contains', 'Archive'],
            ['sg_review_path', 'starts_with', pathStart]
        ]

        return_fields = ['sg_review_path']
        order_by = [{'field_name': 'updated_at', 'direction': 'asc'}]

        def _results_ready(result):

            if not result:
                return

            ri = RlpCore.CoreDs_ModelDataItem.makeItem()
            for playlistEntry in sorted(result, key=lambda x:x['sg_review_path']):

                playlistFullPath = playlistEntry['sg_review_path']
                pi = RlpCore.CoreDs_ModelDataItem.makeItem()

                playlistName = playlistFullPath.replace(pathStart, '')
                pi.insert('id', playlistEntry['id'])
                pi.insert('path', playlistName)
                pi.insert('parent', data['path'])

                ri.append(pi)

            item.insertItem('__children__', ri)
            item.setPopulated()

        self.edbc.ioclient('sg').call(_results_ready, 'Playlist',
            filters, return_fields, order=order_by).run()


    def requestPlaylistContents(self, callback):

        self.edbc.ioclient('sg').call(callback,
            'playlist.get_playlist_versions',
            self.playlistView.project_entity,
            playlist_path=self.playlistView.selReview
        ).run()

