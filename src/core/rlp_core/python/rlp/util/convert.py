
import os

_CONVERT = None

def _initConvert():
    print('Initializing utility convert class')

    global _CONVERT
    import RlpCoreUTILmodule
    _CONVERT = RlpCoreUTILmodule.UTIL_Convert()



def b64decode(encodedInput):

    global _CONVERT

    if os.getenv('RLP_APP_PLATFORM') == 'ios':
        return _CONVERT.b64decode(encodedInput)


    else:
        import base64
        return base64.b64decode(encodedInput)



def jsonLoads(jsonStr):

    global _CONVERT

    if os.getenv('RLP_APP_PLATFORM') == 'ios':
        return _CONVERT.jsonLoads(jsonStr)
    
    else:
        import json
        return json.loads(jsonStr)


_initConvert()
del _initConvert