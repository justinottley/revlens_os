'''
Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
'''


DEFAULT = {
    'left': {u'text': '', u'modifiers': 536870912, u'key': 16777234},
    'right': {u'text': '', u'modifiers': 536870912, u'key': 16777236},
    'space': {u'text': u' ', u'modifiers': 0, u'key': 32},
    'esc': {u'text': '\x1b', u'modifiers': 0, u'key': 16777216},
    'tab': {u'text': '\t', u'modifiers': 0, u'key': 16777217},
    'a': {'text': 'a', 'modifiers':0, 'key': 65},
    'b': {'text': 'b', 'modifiers':0, 'key': 66},
    'c': {u'text':'c', 'modifiers':0, 'key':67},
    'd': {'text': 'd', 'modifiers':0, 'key': 68},
    'e': {'text':'e', 'modifiers':0, 'key':69},
    'f': {u'text':'f', 'modifiers':0, 'key':70},
    'g': {'text': 'g', 'modifiers':0, 'key': 71},
    'h': {u'text':'h', u'modifiers':0, u'key': 72},
    'i': {'text': 'i', 'modifiers':0, 'key': 73},
    'j': {'text': 'j', 'modifiers':0, 'key': 74},
    'k': {'text': 'k', 'modifiers':0, 'key': 75},
    'r': {'text': 'r', 'modifiers':0, 'key': 82},
    's': {'text': 's', 'modifiers':0, 'key': 83},
    't': {'text': 't', 'modifiers':0, 'key': 84},
    '[': {'text': '[', 'modifiers':0, 'key': 91},
    ']': {'text': ']', 'modifiers':0, 'key': 93},
    '\\': {'text': '\\', 'modifiers':0, 'key': 92},
    '1': {'text': '1', 'modifiers':0, 'key': 49},
    '2': {'text': '2', 'modifiers':0, 'key': 50},
    '3': {'text': '3', 'modifiers':0, 'key': 51},
    '4': {'text': '4', 'modifiers':0, 'key': 52},
    '5': {'text': '5', 'modifiers':0, 'key': 53},
    '6': {'text': '5', 'modifiers':0, 'key': 54},
    '7': {'text': '5', 'modifiers':0, 'key': 55},
    '8': {'text': '5', 'modifiers':0, 'key': 56},
    '9': {'text': '5', 'modifiers':0, 'key': 57},
    '0': {'text': '5', 'modifiers':0, 'key': 48},
}


LINUX = {
    'left': {u'text': '', u'modifiers': 0, u'key': 16777234},
    'right': {u'text': '', u'modifiers': 0, u'key': 16777236},
}


from . import KeyEntry

def register_keys():
    
    for name, key_data in DEFAULT.items():
        KeyEntry.register_key_entry(name, **key_data)
        
        
    for name, key_data in LINUX.items():
        KeyEntry.register_key_entry(name, platform='Linux', **key_data)
        KeyEntry.register_key_entry(name, platform='Windows', **key_data)