


from rlplug_shotgrid.sg_handler import ShotgunHandler

def get_playlist_versions(
        project,
        playlist_id=None,
        playlist_path=None,
        SG=None,
        run_id=None):

    if not SG:
        SG = ShotgunHandler.instance()


    print('{} {} {}'.format(project, playlist_path, SG))

    playlist_entity = None
    if playlist_id:
        playlist_entity = {'type': 'Playlist', 'id': playlist_id}

    else:
        pl_result = SG.find('Playlist',
            [['project', 'is', project],
             ['sg_review_path', 'is', playlist_path]],
            ['id', 'code']
        )

        print(pl_result)
        if not pl_result:
            raise Exception('No playlist found for path {}'.format(playlist_path))

        for pl_entry in pl_result:
            if (len(pl_entry['code'].split('-')) < 3):
                
                playlist_entity = pl_entry

                print('FOUND PLAYLIST: {}'.format(playlist_entity))
                break


        if not playlist_entity:
            raise Exception('Could not get a current playlist')


    print('searching {} {}'.format(project, playlist_entity))
    ver_result = SG.find('Version',
        # ['project', 'is', project],
        [['playlists', 'is', playlist_entity]],
        ['code', 'sg_path_to_frames', 'frame_count', 'description', 'user']
    )
    
    print('GOT VERSIONS:')
    print(ver_result)

    return ver_result




