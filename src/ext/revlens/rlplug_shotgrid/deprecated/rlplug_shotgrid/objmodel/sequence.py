#
# Copyright 2024 Justin Ottley
#
# Licensed under the terms set forth in the LICENSE.txt file
#

import pprint

from . import SGObject
from .version import SGVersion

class SGSequence(SGObject):
    _SEQ_CACHE = {
        'id': {},
        'name': {}
    }

    def __init__(self, sg, project, sg_id=None, name=None):
        SGObject.__init__(self, sg, project)

        return_fields = ['act', 'shots', 'code', 'sg_pipeline_name']

        sg_info = None
        self.__shot_id_map = {}
        self.__shot_name_map = {}
        self.__shot_id_list = []


        if sg_id:
            sg_info = self.sg.find('Sequence',
                [['project', 'is', self.project],
                 ['id', 'is', sg_id]],
                return_fields
            )

        elif name:
            sg_info = self.sg.find('Sequence',
                [['project', 'is', self.project],
                 ['code', 'is', name]],
                return_fields
            )

        assert(len(sg_info) == 1)
        self._sg_info = sg_info[0]

        shot_id_list = [shot_entry['id'] for shot_entry in sg_info[0]['shots']]

        self.shot_list = self.sg.find('Shot',
            [
                ['sg_sequence', 'is', self.sg_hash],
                ['id', 'in', shot_id_list]
            ],
            ['code', 'sg_cut_order', 'sg_cut_in', 'sg_cut_out', 'sg_head_in', 'sg_head_out'],
            order=[{'field_name': 'sg_cut_order', 'direction': 'asc'}]
        )
        
        
        for shot_entry in self.shot_list: # self._sg_info['shots']:

            self.logger.debug(pprint.pformat(shot_entry))

            sg_shot_obj = SGShot(self, shot_entry)

            self.__shot_id_map[shot_entry['id']] = sg_shot_obj
            self.__shot_name_map[shot_entry['code']] = sg_shot_obj
            self.__shot_id_list.append(shot_entry['id'])

    @property
    def name(self):
        return self._sg_info['code']

    @property
    def sg_id(self):
        return self._sg_info['id']

    @property
    def sg_hash(self):
        return {'type': 'Sequence', 'id': self.sg_id}

    @property
    def pipeline_name(self):
        return self._sg_info['sg_pipeline_name']


    @classmethod
    def make_sequence(cls, sg, project, sg_id=None, name=None):
        
        seq_obj = None

        if sg_id:
            if sg_id in cls._SEQ_CACHE['id']:
                seq_obj = cls._SEQ_CACHE['id'][sg_id]

            else:
                seq_obj = SGSequence(sg, project, sg_id=sg_id, name=name)

                cls._SEQ_CACHE['id'][seq_obj.sg_id] = seq_obj
                cls._SEQ_CACHE['name'][seq_obj.name] = seq_obj

        elif name:
            if name in cls._SEQ_CACHE['name']:
                seq_obj = cls._SEQ_CACHE['name'][name]

            else:
                seq_obj = SGSequence(sg, project, sg_id=sg_id, name=name)

                cls._SEQ_CACHE['id'][seq_obj.sg_id] = seq_obj
                cls._SEQ_CACHE['name'][seq_obj.name] = seq_obj

        return seq_obj


    def _get_shot_idx(self, sg_id=None, name=None):
        if name:
            sg_id = self.__shot_name_map[name].sg_id

        shot_idx = self.__shot_id_list.index(sg_id)
        return shot_idx

    def next_shot(self, sg_id=None, name=None):
        shot_idx = self._get_shot_idx(sg_id=sg_id, name=name)
        next_shot_idx = shot_idx + 1
        if len(self.__shot_id_list) > next_shot_idx:
            return self.__shot_id_map[self.__shot_id_list[next_shot_idx]]

    def prev_shot(self, sg_id=None, name=None):
        shot_idx = self._get_shot_idx(sg_id=sg_id, name=name)
        prev_shot_idx = shot_idx - 1
        if prev_shot_idx >= 0:
            return self.__shot_id_map[self.__shot_id_list[prev_shot_idx]]
        

class SGShot(SGObject):

    def __init__(self, sequence, shot_info):
        SGObject.__init__(self, sequence.sg, sequence.project)
        self.sequence = sequence

        self._sg_info = shot_info

    @property
    def name(self):
        for key in ['code', 'name']:
            if key in self._sg_info:
                return self._sg_info[key]

    @property
    def sg_id(self):
        return self._sg_info['id']

    @property
    def sg_entity(self):
        return {'type': 'Shot', 'id': self.sg_id}

    def next(self):
        return self.sequence.next_shot(self.sg_id)

    def get_best_version(self, dept):
        
        self.logger.info('get_best_version(): {}'.format(dept))

        versions = self.sg.find('Version',
            [['entity', 'is', self.sg_entity],
             ['sg_department', 'is', dept]
            ],
            SGVersion.VERSION_FIELDS
        )

        # self.logger.debug(pprint.pformat(versions))

        return versions[0]
        # for version_entry in versions:
