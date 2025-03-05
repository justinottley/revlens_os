#
# Copyright 2024 Justin Ottley
#
# Licensed under the terms set forth in the LICENSE.txt file
#

import os
import time
import pprint

from ..objmodel.sequence import SGSequence
from ..objmodel.version import SGVersion

class SequenceMixin:

    def make_sequence(self, project, sg_id=None, name=None):
        project_hash = self.make_project(project)
        return SGSequence.make_sequence(self.sg, project_hash, sg_id=sg_id, name=name)


    def get_shot_sequence(self, project, sg_id=None, name=None):

        self.logger.debug('get_shot_sequence(): {} sg_id: {} name: {}'.format(
            project, sg_id, name
        ))
        
        project_hash = self.make_project(project)

        filters = [['project', 'is', project_hash]]
        if sg_id:
            filters.append(['id', 'is', sg_id])

        if name:
            filters.append(['code', 'is', name])

        shot_info = self.sg.find('Shot',
            filters,
            ['sg_sequence']
        )

        if shot_info and len(shot_info) == 1:
            return shot_info[0]['sg_sequence']


    def get_shots(self, seq_id):
        
        # Cache shots in sequence
        #
        if seq_id not in self.SEQ_CACHE:
            shots = self.sg.find(
                'Shot',
                [['sg_sequence', 'is', {'type': 'Sequence', 'id': seq_id}]],
                ['code', 'sg_cut_order', 'sg_cut_in', 'sg_cut_out', 'sg_head_in', 'sg_head_out'],
                order=[{'field_name': 'sg_cut_order', 'direction': 'asc'}]
            )

            if shots:

                self.logger.debug('Caching shots for Sequence ID {}'.format(seq_id))
                self.SEQ_CACHE[seq_id] = {
                    'shots': shots
                }

        return self.SEQ_CACHE[seq_id]['shots']


    def build_media_list_from_sequence(self, seq_id):
        
        seq_obj = self.make_sequence('stick', sg_id=seq_id)
        
        if not seq_obj:
            self.logger.error('No sequences found with ID {}'.format(seq_id))
            return

        seq_name = seq_obj.pipeline_name
        self._display_progress_message('Loading Sequence {} ..'.format(seq_name))

        self.logger.info('build_sequence(): {} - {}'.format(seq_id, seq_name))

        shots = seq_obj.shot_list

        cut_order_map = {}
        shot_idx = 1
        shot_count = len(shots)

        for shot in shots:
            
            # self.logger.debug(pprint.pformat(shot))

            # Skip MSA shot
            #
            if '_000' in shot['code']:
                continue
                
            cut_order_map[shot['sg_cut_order']] = shot
            
            percent_complete = int((float(shot_idx) / float(shot_count)) * 100)
            
            self._display_progress_message('Getting Versions for {s} .. {p}% - ( {n} / {t} )'.format(
                s=shot['code'], n=shot_idx, t=shot_count, p=percent_complete))

            versions = self.sg.find(
                'Version',
                [['entity', 'is', shot]],
                SGVersion.VERSION_FIELDS
                )
            
            ver_time_map = {}
            
            for ver_info in versions:
                
                # self.logger.debug(pprint.pformat(ver_info))
                # self.logger.debug('')
                
                if ver_info['sg_department'] in ['Lighting', 'Comp', 'Fx']:
                   self.logger.debug('SKIPPING {v}'.format(v=ver_info['code']))
                   continue
                    
                epoch = time.mktime(ver_info['created_at'].timetuple())
                ver_time_map[epoch] = ver_info
                
                
            # self.logger.info(versions)
            shot['versions'] = ver_time_map
            # self.logger.debug(pprint.pformat(ver_time_map))
            
            shot_idx += 1
        
        rl_media_list = []
        
        for shot_co in sorted(cut_order_map):
            shot = cut_order_map[shot_co]
            versions = shot['versions']
            
            if versions:
                
                latest_ver = versions[max(versions.keys())]
                
                media_builder = self.version_make_media(latest_ver)
                rl_media_list.append(media_builder.payload)



        return (rl_media_list, seq_name)

