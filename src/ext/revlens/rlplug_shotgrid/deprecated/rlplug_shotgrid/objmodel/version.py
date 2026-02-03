#
# Copyright 2024 Justin Ottley
#
# Licensed under the terms set forth in the LICENSE.txt file
#

class SGVersion(object):

    VERSION_FIELDS = [
        'project',
        'code',
        'entity',
        'sg_path_to_movie',
        'sg_path_to_frames',
        'sg_department',
        'sg_status_list',
        'sg_uploaded_movie',
        'created_at',
        'sg_first_frame',
        'sg_last_frame',
        'description',
        'user',
        'image',
        'sg_task'
    ]


    @classmethod
    def get_media_metadata(cls, sg_version_info):

        user_result =  sg_version_info.get('user') or {}
        username = user_result.get('name', '')

        task_result = sg_version_info.get('task') or {}
        task = task_result.get('name', '')

        result = {
            sg_version_info['entity']['type'].lower():sg_version_info['entity']['name'],
            'entity.name': sg_version_info['entity']['name'],
            'user': username,
            'task': task,
            'created_at': str(sg_version_info['created_at']).replace('-07:00', ''),
            'description': sg_version_info['description'],
            'status': sg_version_info['sg_status_list'],
            'department': sg_version_info['sg_department'],
            'code': sg_version_info['code'],
            'project.sg_id': sg_version_info['project']['id'],
            'first_frame': sg_version_info['sg_first_frame'],
            'last_frame': sg_version_info['sg_last_frame']
        }

        return result
