#
# Copyright 2024 Justin Ottley
#
# Licensed under the terms set forth in the LICENSE.txt file
#

import logging

class SGObject(object):
    def __init__(self, sg, project):

        self.logger = logging.getLogger('rlp.{ns}.{cls}'.format(
            ns=self.__class__.__module__, cls=self.__class__.__name__
        ))

        self.sg = sg
        self.project = project
