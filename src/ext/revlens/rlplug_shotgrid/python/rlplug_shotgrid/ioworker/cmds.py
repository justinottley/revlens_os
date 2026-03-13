
import logging

from rlplug_shotgrid.sg_handler import ShotgunHandler

LOG = logging.getLogger('rlp.{}'.format(__name__))


def find(entityType, filters, returnFields, **kwargs):

    if 'run_id' in kwargs:
        del kwargs['run_id']


    print('find(): {} {} {}'.format(entityType, filters, returnFields))

    SG = ShotgunHandler.instance()
    return SG.find(entityType, filters, returnFields, **kwargs)


def get_shots_by_sequence(project_id, seq_code=None, seq_id=None, **kwargs):

    SG = ShotgunHandler.instance()

    run_id = kwargs['run_id']

    project_entity = {'type': 'Project', 'id': project_id}

    LOG.progress(run_id, 'Getting Shots for Sequence "{}"'.format(seq_code))

    # Strictly speaking not really necessary since
    # we can do namespaced style: sg_sequence.Sequence.code
    #
    if seq_code and not seq_id:
        seq_result = SG.find('Sequence', [
            ['project', 'is', project_entity],
            ['code', 'is', seq_code]
        ])
        if seq_result and len(seq_result) == 1:
            seq_id = seq_result[0]['id']

            LOG.progress(run_id, 'Got Sequence ID {} for Sequence {}'.format(seq_id, seq_code))

    shot_result = []

    if not seq_id:
        LOG.progress(run_id, 'Error: no sequence id, aborting query')
        return shot_result

    
    if seq_result and len(seq_result) == 1:

        seq_id = seq_result[0]['id']
        seq_entity = {'type': 'Sequence', 'id': seq_id}

        shot_result = SG.find('Shot',
            [['project', 'is', project_entity],
            ['sg_sequence', 'is', seq_entity],
            ['sg_status_list', 'is_not', 'oop']],
            ['code', 'image', 'sg_sequence'],
            order=[{'field_name': 'sg_orderno', 'direction': 'asc'}]
        )


    return shot_result


def get_sequence_media(project_id, sequence_id, media_callback=None, **kwargs):

    SG = ShotgunHandler.instance()

    project_entity = {'type': 'Project', 'id': project_id}
    seq_entity = {'type': 'Sequence', 'id': sequence_id}

    run_id = kwargs['run_id']
    LOG.progress(run_id, 'Getting Shot List..')

    shot_result = SG.find('Shot',
        [['project', 'is', project_entity],
         ['sg_sequence', 'is', seq_entity],
         ['sg_status_list', 'is_not', 'oop']],
         ['code', 'image', 'sg_sequence'],
        order=[{'field_name': 'sg_orderno', 'direction': 'asc'}]
    )

    shot_media_list = []

    pref_dept_list = kwargs.get('pref_dept_list')

    shot_count = len(shot_result)

    shot_idx = 1
    for shot_entry in shot_result:

        # print(shot_entry['code'])
        LOG.progress(run_id, shot_entry['code'], {'count': shot_count, 'idx': shot_idx})

        ver_filters = []
        if kwargs.get('filters'):
            ver_filters += kwargs['filters']

        ver_filters.append(
            ['entity', 'is', {'type': 'Shot', 'id': shot_entry['id']}]
        )


        # print('')
        #
        # TODO FIXME: inject site specific return fields
        #

        ver_result = SG.find('Version',
            ver_filters,
            ['sg_path_to_movie', 'sg_path_to_frames', 'sg_department', 'sg_final',
             'code', 'user', 'image', 'frame_count'],
            order=[{'field_name': 'created_at', 'direction': 'desc'}]
        )

        if ver_result:
            use_ver = None
            if pref_dept_list:
                for dept_pref in pref_dept_list:

                    # print('')
                    # print('------=== CHECKING : {} ===------'.format(dept_pref))
                    # print('')

                    if use_ver:
                        break
                    
                    for ver_entry in ver_result:

                        if ver_entry['sg_department'] == dept_pref:

                            if kwargs.get('pref_order') == 'most_recent':
                                use_ver = ver_entry
                                break

                            elif kwargs.get('pref_order') == 'final' and ver_entry['sg_final']:
                                use_ver = ver_entry
                                break
                if not use_ver:
                    print('{}: WARNING: unable to locate a version, falling back to latest'.format(shot_entry['code']))
                    use_ver = ver_result[0]

            else:
                use_ver = ver_result[0]


            mediaName = use_ver['code']
            mediaMd = use_ver


            media_result = {
                'media.video': use_ver['sg_path_to_frames'],
                'media.name': mediaName,
                'media.frame_count': use_ver['frame_count'],
                'media.metadata': mediaMd
            }
            shot_media_list.append(media_result)

            shot_idx += 1


    return shot_media_list

