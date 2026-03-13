#
# Copyright 2024 Justin Ottley
#
# Licensed under the terms set forth in the LICENSE.txt file
#

def cbFormatMediaData(versionInfo, nameKey='code', mergeMetadata=None):

    mediaName = versionInfo.get(nameKey)
    mediaMd = {
        'user': 'unknown',
        'description': versionInfo.get('description', ''),
        'entity.name': mediaName
    }

    if 'metadata' not in versionInfo:
        versionInfo['metadata'] = {}

    if mergeMetadata and mergeMetadata in versionInfo:
        mediaMd.update(versionInfo[mergeMetadata])

    return (mediaName, mediaMd)
