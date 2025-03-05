
import pprint

import rlp_util.env
rlp_util.env.setup_env(update_sys_path=True)

import rlp_entitydb


def main():

    db_path = '/home/justinottley/dev/revlens_root/sp_revlens/production.db'
    ds_name = 'pdb_test'
    ds_type = 'production_db'

    cntrl = rlp_entitydb.EdbCntrl_Query(db_path, ds_name, ds_type)
    print(cntrl)

    all_types = cntrl.getEntityTypes()
    for entity_type_entry in all_types:

        entity_type_name = entity_type_entry['name']
        entity_fields = cntrl.getEntityTypeFields(entity_type_name)

        print('')
        print('--- {} ---'.format(entity_type_name))
        pprint.pprint(entity_fields)



    print('')
    print('')
    print('---- ENTITIES ----')
    print('')
    print('')



    for entity_entry in all_types:
        entity_type = entity_entry['name']

        print('')
        print('----- {} -----'.format(entity_type))
        print('')

        entity_field_info = cntrl.getEntityTypeFields(entity_type)
        entity_fields = list(entity_field_info.keys())

        print(entity_fields)
        r = cntrl.find(entity_type, [], entity_fields)
        print(r)





if __name__ == '__main__':
    main()
