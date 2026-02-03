
import RlpCorePYmodule
print('hello world')

print(dir(RlpCorePYmodule))

from RlpCorePYmodule import PyQVariant, PyQVariantMap

v = PyQVariant(10)

print(v.toInt())
print(v.pyval())
print('---')

v1 = PyQVariant("test")
print(v1.toString())
print(v1.pyval())
print('---')

m = PyQVariantMap()
m.insert("test", "the")
m.insert("val1", 20.123)
print(m.toDict())

vm = PyQVariant(m)
print(vm.toDict())

vd = PyQVariant(20.123)
print(vd.toDouble())

vb = PyQVariant(True)
print(vb.toBool())


s = {'test': 12, 'the': 123.456, 'val': False, 'strval': 'the quick'}

rm = PyQVariantMap(s)
print(rm.toDict())


s1 = {'key': [1, 2, 3, 'str'], 'another': {'nested': 123.345}}
rml = PyQVariantMap(s1)
print(rml.toDict())


sli = PyQVariant([1, True, 'test', 12.4567])
print(sli.pyval())


slm = PyQVariant(s1)
print(slm.pyval())
