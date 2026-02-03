'''
Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
'''

import json
import base64


def index():
    return base64.b64encode(json.dumps({'retcode': 'ok'}))