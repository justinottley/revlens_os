

import os
import sys
import json
import platform
import datetime

revlens_root = '/home/justinottley/dev/revlens_root'
superproject_root = 'sp_revsix'


def dist_ssets(dist_ver):

    dist_dir = os.path.join(
        revlens_root,
        'dist',
        'revlens',
        dist_ver
    )

    if not os.path.isdir(dist_dir):
        print('Creating {}'.format(dist_dir))
        os.makedirs(dist_dir)


    sset_config_path = os.path.join(
        os.path.dirname(__file__),
        'sc_config_base.json'
    )

    sset_out_path = os.path.join(dist_dir, 'sc_config_{}.json'.format(dist_ver))
    print(sset_config_path)

    with open(sset_config_path) as fh:
        config = json.load(fh)
        config['__bootstrap__']['build_version'] = dist_ver
        config['revlens']['sftset_version'] = dist_ver

        with open(sset_out_path, 'w') as wfh:
            wfh.write(json.dumps(config, indent=2))

            print('Wrote {}'.format(sset_out_path))

    
    cmd_build = 'sset_release --server wss://studio2.revlens.io:8003 build --project revlens --version {} --config {}'.format(
        dist_ver, sset_out_path
    )
    print(cmd_build)

    cmd_app = 'sset_release --server wss://studio2.revlens.io:8003 sset --project revlens --sset revlens --config {}'.format(sset_out_path)
    print(cmd_app)
    
    # cmd_tb = 'sset_release sset --project revlens --sset thirdbase --sset_version 22_09 --config {}'.format(sset_out_path)
    cmd_tb = 'sset_release --server wss://studio2.revlens.io:8003 sset --project revlens --sset thirdbase --config {}'.format(sset_out_path)
    print(cmd_tb)

    cmd_sclib = 'sset_release --server wss://studio2.revlens.io:8003 sset --project revlens --sset thirdbase --config {}'.format(sset_out_path)
    print(cmd_sclib)

    cmd_link_tb = 'sset_release --server wss://studio2.revlens.io:8003 link --project revlens --build_version {} --sset_version 22_09 --sset thirdbase --release_num 1'.format(
        dist_ver
    )
    print(cmd_link_tb)

    cmd_link_app = 'sset_release --server wss://studio2.revlens.io:8003 link --project revlens --build_version {} --sset_version {} --sset revlens --release_num 1'.format(
        dist_ver, dist_ver
    )
    print(cmd_link_app)




def main():

    if len(sys.argv) > 1:
        dist_ver = sys.argv[1]
    else:
        now = datetime.datetime.today()
        dist_ver = '{y}.{m}.{d}'.format(y=str(now.year)[2:], m=str(now.month).zfill(2), d=str(now.day).zfill(2))

    # os.environ['D

    print('Dist Ver: {}'.format(dist_ver))
    dist_ssets(dist_ver)
    print('Done')

if __name__ == '__main__':
    main()
