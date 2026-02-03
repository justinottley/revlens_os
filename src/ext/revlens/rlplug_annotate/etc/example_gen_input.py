
import json
import base64

result = base64.b64encode(json.dumps({
    'type': 'colour',
    'resolution.x': 1920,
    'resolution.y': 1080,
    'colour': 'transparent'
    }).encode('utf-8'))

print(result)