'''
Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
'''

import logging

from rlp.util import buildopt
import rlp.core.util as core_util
import rlp.gui as RlpGui

LOG = logging.getLogger('rlp.{ns}'.format(ns=__name__))

class KeyEntry(object):
    
    KEY_MAP = {}
    OPSYS = buildopt.get_buildopt('opsys')
    
    def __init__(self, name, text, modifiers, key, platform=None):
        
        self.name = name
        
        self._platforms = {
            'Linux': {},
            'Darwin': {},
            'Windows': {},
            'Emscripten': {},
            '__default__': {}
        }
        
        self.update(text, modifiers, key, platform)


    def update(self, text, modifiers, key, platform=None):
        
        plat_key = platform if platform else '__default__'
        self._platforms[plat_key] = {
            'text': text,
            'modifiers': modifiers,
            'key': key
        }


    def match(self, event):

        plat_key = '__default__'

        if self.OPSYS not in self._platforms:
            LOG.warning('Unknown platform: {}'.format(self.OPSYS))

        elif self._platforms.get(self.OPSYS):
            plat_key = self.OPSYS

        key_info = self._platforms[plat_key]

        return ((key_info['key'] == event['key']) and \
                (key_info.get('modifiers', 0) == event.get('modifiers', 0)) and \
                (key_info['text'] == event['text']))


    @classmethod
    def register_key_entry(cls, name, text, modifiers, key, platform=None):

        plat_name = platform if platform else '__default__'

        # LOG.verbose('key: [ {plat} ] : "{n}" - text: {t} modifiers: {m} key: {k}'.format(
        #     plat=plat_name, n=name, t=text, m=modifiers, k=key))

        kentry = cls.KEY_MAP.get(name)

        if kentry:
            kentry.update(text, modifiers, key, platform)

        else:

            ke = KeyEntry(name, text, modifiers, key, platform)
            cls.KEY_MAP[name] = ke


class KeyBinding(object):

    KEY_BINDINGS = []

    def __init__(self, key_entry_name, func_name, description=None, widgetItem=None):

        self.key_entry_name = key_entry_name
        self.func_name = func_name
        self.func = None
        if func_name:
            if type(func_name) == str:
                self.func = core_util.import_function(func_name)
            elif callable(func_name):
                self.func = func_name

        self.description = description or func_name
        self.widgetItem = widgetItem

    @classmethod
    def register(cls, key_entry_name, func_name, description=None, widgetItem=None):

        kb = KeyBinding(key_entry_name, func_name, description=description, widgetItem=widgetItem)
        KeyBinding.KEY_BINDINGS.append(kb)


    def run(self):

        # LOG.verbose('Running {func}'.format(func=self.func))
        self.func()


def register_key_entry(name, text, modifiers, key, platform=None):
    return KeyEntry.register_key_entry(name, text, modifiers, key, platform)


def register_key_binding(key_binding):
    KeyBinding.KEY_BINDINGS.append(key_binding)


def handle_key_press_event(key_event, *args):

    if (RlpGui.VIEW.hasFocusItem()):
        key_event['widgetItem'] = RlpGui.VIEW.focusItem()

    for binding in KeyBinding.KEY_BINDINGS:

        if match(binding, key_event):

            # LOG.verbose('matched {k} to {n}'.format(k=key_event, n=binding.key_entry_name))

            result = binding.run()
            return


def getKeystroke(name):
    return KeyEntry.KEY_MAP[name]


def match(binding, event):

    # print(event)

    eventWidgetItem = event.get('widgetItem')

    # if there is a focus item and the binding doesn't supply one,
    # count this as a mismatch
    if eventWidgetItem and not binding.widgetItem:
        return False

    # if there is a specific widget that is targetted for this binding,
    # check that first
    if binding.widgetItem:
        if not eventWidgetItem:

            LOG.debug('match(): no focused widget, binding expects {}'.format(
                binding.widgetItem))

            return False

        elif binding.widgetItem != eventWidgetItem:
            LOG.debug('match(): focus widget mismatch, binding expects {}, current focus: {}'.format(
                binding.widgetItem, eventWidgetItem))
            
            return False

    key_event = KeyEntry.KEY_MAP.get(binding.key_entry_name)
    if key_event:
        return key_event.match(event)

    else:
        LOG.debug('match(): no key entry for {}'.format(binding.key_entry_name))
        return False
