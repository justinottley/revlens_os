
import sys
import uuid
import pprint

import rlp_core
rlp_core.init_standalone()

from rlp_core import EdbCntrl_Query, EdbDs_DataSource, QUuid




def test_find():

    q = EdbCntrl_Query('test.db')
    
    print(q.find(
        'VideoStream',
        [['name', 'is', 'main'], ['frame_count', 'is', 30]],
        ['codec_short', 'frame_count', 'name']))


def test_create_entity():

    # ds = EdbDs_DataSource('rlmedia', 'media_container')
    
    q = EdbCntrl_Query('test.db', 'rlmedia', 'media_container')
    print(q)
    
    print(q.getEntityTypes())

    return

    vs = q.createEntityType('VideoStream')
    f = vs.addField(
        "codec_short",
        "Codec Short Name",
        "str"
    )

def test_create_adb():

    q = EdbCntrl_Query('production.db', 'pdb_test', 'production_db')

    etypes = {
        'HumanUser': [
            {
                'name': 'username',
                'display_name': 'Username',
                'type': 'str'
            },
            {
                'name': 'email',
                'display_name': 'Email',
                'type': 'str'
            },
            {
                'name': 'first_name',
                'display_name': 'First Name',
                'type': 'str'
            },
            {
                'name': 'last_name',
                'display_name': 'Last Name',
                'type': 'str'
            }
        ],
        'Project': [
            {
                'name': 'fullname',
                'display_name': 'Full Name',
                'type': 'str'
            }
        ],
        'Asset': [
            {
                'name': 'project',
                'display_name': 'Project',
                'type': 'entity',
                'metadata': {'entity_types': ['Project']}
            },
            {
                'name': 'geometry',
                'display_name': 'Geo',
                'type': 'entity',
                'metadata': {'entity_types': ['Geo']}
            },
            {
                'name': 'rig',
                'display_name': 'Rig',
                'type': 'entity',
                'metadata': {'entity_types': ['Rig']}
            }
        ],
        'Rig': [
        ],
        'Sequence': [
            {
                'name': 'project',
                'display_name': 'Project',
                'type': 'entity',
                'metadata': {'entity_types': ['Project']}
            },
            {
                'name': 'seq_num',
                'display_name': 'Sequence Number',
                'type': 'str'
            },
            {
                'name': 'seq_name',
                'display_name': 'Sequence Name',
                'type': 'str'
            }
        ],
        'Shot': [
            {
                'name': 'project',
                'display_name': 'Project',
                'type': 'entity',
                'metadata': {'entity_types': ['Project']}
            },
            {
                'name': 'sequence',
                'display_name': 'Sequence',
                'type': 'entity',
                'metadata': {'entity_types': ['Sequence']}
            }
        ],
        'Task': [
            {
                'name': 'link',
                'display_name': 'Link',
                'type': 'entity',
                'metadata': {'entity_types': ['Shot']}
            }
        ],
        'Note': [
            {
                'name': 'link',
                'display_name': 'Link',
                'type': 'entity',
                'metadata': {'entity_types': ['Shot']}
            }
        ],
        'Take': [
            {
                'name': 'link',
                'display_name': 'Link',
                'type': 'entity',
                'metadata': {'entity_types': ['Shot','Asset']}
            }
        ]
    }

    for etype_name, etype_fields in etypes.items():

        print(etype_name)
        print(etype_fields)
        print('')

        et = q.createEntityType(etype_name)

        for field_data in etype_fields:
            f = et.addField(
                field_data['name'],
                field_data['display_name'],
                field_data['type']
            )

            if 'metadata' in field_data:
                f.setMetadata(field_data['metadata'])

def test_create_entity_sequence_adb():

    q = EdbCntrl_Query('production.db', 'pdb_test', 'production_db')

    # r = q.create('Project', 'stick', {'fullname': 'The Stick Games'})

    s = q.create(
        'Sequence',
        '010_Intro',
        {
            'project': QUuid('{840df142-5a5b-46e7-b9ff-337a3c1ae377}'),
            'seq_num': '010',
            'seq_name': 'Intro'
        }
    )


    print(s)

def test_create_entity_shot_adb():

    q = EdbCntrl_Query('production.db', 'pdb_test', 'production_db')

    # r = q.create('Project', 'stick', {'fullname': 'The Stick Games'})

    s = q.create(
        'Shot',
        '010_020',
        {
            'project': QUuid('{840df142-5a5b-46e7-b9ff-337a3c1ae377}'),
            'sequence': QUuid('{e9c461ac-78d3-4549-a846-737e62483c08}'),
        }
    )


    print(s)


def test_find_adb():

    q = EdbCntrl_Query('production.db', 'pdb_test', 'production_db')


    r = q.find('Shot', [], ['name', 'project', 'sequence'])

    print(r)
    
    # print(q.find('Sequence', [['name', 'is', '010_Intro']], ['seq_name', 'seq_num', 'project']))


def test_etypes():

    q = EdbCntrl_Query(sys.argv[1])

    etypes = q.getEntityTypes()

    print(etypes)
    for et in etypes:

        pprint.pprint(q.getEntityTypeFields(et['name']))


def test_create_etype():

    q = EdbCntrl_Query(sys.argv[2], 'rlmedia', 'media_container')

    et = q.createEntityType('VideoStream')
    f = et.addField(
        "codec_short",
        "Codec Short Name",
        "str"
    )

    et = q.createEntityType('Thumbnail')
    field = et.addField(
        'video_stream',
        'Video Stream',
        'entity'
    )
    field.setMetadata({'entity_types': ['VideoStream']})

    print(et)
    print(type(et))
    # print dir(et)

    print(f)
    print(type(f))



def main():

    test_name = sys.argv[1]
    eval('test_{}'.format(test_name))()

if __name__ == '__main__':
    main()

