
import logging
from rlp.core import rlp_logging

rlp_logging.basic_config(loglevel=logging.VERBOSE)

import scaffold.env
scaffold.env._init()

import rlp.core
print(rlp.core)

from rlp.core.path import Path

p = Path('thirdbase://')
p.path_info
print(str(p))


p = Path('dist://')
print(str(p))