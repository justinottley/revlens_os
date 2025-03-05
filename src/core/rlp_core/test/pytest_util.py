
import RlpCorePYmodule
print(dir(RlpCorePYmodule))

from RlpCorePYmodule import QString

s = QString("hello")
print(s)

import RlpCoreUTILmodule

print(dir(RlpCoreUTILmodule))
from RlpCoreUTILmodule import UTIL_Text

t = UTIL_Text()
print(t)
t.getTextWidth(QString("hello"), QString(""))

print(t)