'''
Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
'''

import sys

from pybindgen.typehandlers import base as typehandlers
from pybindgen import ReturnValue, Parameter, Module, FileCodeSink


def get_module():

    mod = Module('libRlpRevlensDSmodule')
    
    
    mod.add_include('"RlpRevlens/DS/Json.h"')
    
    cls = mod.add_class('DS_JsonObject')
    
    typehandlers.add_type_alias('std::vector<DS_JsonObject>', 'DS_JsonList')
    typehandlers.add_type_alias('std::vector<DS_JsonObject>&', 'DS_JsonList&')
    
    typehandlers.add_type_alias('std::map<const char*, DS_JsonObject>', 'DS_JsonDict')
    typehandlers.add_type_alias('std::map<const char*, DS_JsonObject>&', 'DS_JsonDict&')
    
    
    mod.add_container('std::vector<DS_JsonObject>',
                      ReturnValue.new('DS_JsonObject'),
                      'vector')
    
    mod.add_container('std::map<std::string, DS_JsonObject>',
                      (ReturnValue.new('std::string'), ReturnValue.new('DS_JsonObject')),
                      'map')
    
    mod.add_container('DS_JsonDict',
                      (ReturnValue.new('std::string'), ReturnValue.new('DS_JsonObject')),
                      'map')
    
    mod.add_container('DS_JsonList',
                      ReturnValue.new('DS_JsonObject'),
                      'vector')
    
    cls.add_enum('JSON_TYPE', 
        ['JSON_TYPE_NULL',
         'JSON_TYPE_INT',
         'JSON_TYPE_FLOAT',
         'JSON_TYPE_STRING',
         'JSON_TYPE_LIST',
         'JSON_TYPE_DICT',
         'JSON_TYPE_UNSUPPORTED'])
    
    cls.add_constructor([])
    cls.add_constructor([Parameter.new('int', 'int_val')])
    cls.add_constructor([Parameter.new('int', 'int_val'),
                         Parameter.new('bool', 'as_json_type')])
    cls.add_constructor([Parameter.new('float', 'float_val')])
    cls.add_constructor([Parameter.new('std::string', 'string_val')])
    cls.add_constructor([Parameter.new('DS_JsonList', 'list_val')])
    # cls.add_constructor([Parameter.new('std::map<std::string, DS_JsonObject>', 'map_val')])
    cls.add_constructor([Parameter.new('DS_JsonDict', 'map_val2')])
    
    cls.add_method('get_int', ReturnValue.new('int'), [])
    cls.add_method('get_float', ReturnValue.new('float'), [])
    cls.add_method('get_string', ReturnValue.new('std::string'), [])
    cls.add_method('get_list', ReturnValue.new('std::vector<DS_JsonObject>'), [])
    cls.add_method('get_map', ReturnValue.new('std::map<std::string, DS_JsonObject>'), [])
    
    
    cls.add_method('set_data', 'void',
                    [Parameter.new('const char*', 'key'),
                     Parameter.new('DS_JsonObject&', 'value')])
    
    cls.add_method('append', 'void',
                   [Parameter.new('DS_JsonObject&', 'value')])
    
    cls.add_method('get_type', ReturnValue.new('JSON_TYPE'), [])
    
    cls.add_method('get_data', ReturnValue.new('DS_JsonObject'), [Parameter.new('const char*', 'key')])
    
    # cls.add_method('getStringData', ReturnValue.new('std::string'), [Parameter.new('const char*', 'key')])
    # cls.add_method('getValue', ReturnValue.new('DS_JsonObject'), [Parameter.new('const char*', 'key')])
    
    return mod
