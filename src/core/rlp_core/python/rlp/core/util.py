
import os
import re
import sys
import json
import pprint
import logging
import traceback
from types import ModuleType

from functools import reduce

rlogger = logging.getLogger('rlp.{name}'.format(name=__name__))

def import_module(module_ns):

    top_module = __import__(module_ns)

    for entry in module_ns.split('.')[1:]:
        top_module = getattr(top_module, entry)

    return top_module


def import_function(func_namespace):

    func_parts = func_namespace.split('.')
    module_ns = func_parts[0:-1]
    func_name = func_parts[-1]

    module = import_module('.'.join(module_ns))

    return getattr(module, func_name)


def loft_to_module(module_name, obj, skip=None):

    skip = skip or ['bootstrap', 'init_module']

    for entry in dir(obj):

        entry_obj = getattr(obj, entry)

        if entry.startswith('_'):
            continue

        if entry in skip:
            continue

        if isinstance(entry_obj, ModuleType):
            continue

        # print('{}: adding {}'.format(module_name, entry))
        setattr(sys.modules[module_name], entry, entry_obj)


def py_call(callable, args, kwargs):
    '''
    A helper / wrapper for calling python functions from C++
    '''
    return callable(*args, **kwargs)


def json_call(json_str):
    '''
    TODO FIXME: DEPRECATED
    '''

    envelope = {'status': 'UNKNOWN'}
    call_data = None

    try:
        call_data = json.loads(json_str)

        func_ns = call_data['method']
        func_args = call_data.get('args', [])
        func_kwargs = call_data.get('kwargs', {})

        func_obj = import_function(func_ns)

        # print('Calling {} args: {} kwargs: {}'.format(
        #     func_ns, func_args, func_kwargs
        # ))

        raw_result = func_obj(*func_args, **func_kwargs)

        # print('raw result: {}'.format(raw_result))

        envelope = {
            'status': 'OK',
            'result': raw_result
        }

        # print('json_call: OK')
        # pprint.pprint(envelope)

    except:

        tb_str = traceback.format_exc()
        envelope = {
            'status': 'ERR',
            'err_msg': tb_str
        }

        print('json_call failed')
        print(tb_str)
        pprint.pprint(call_data)


    json_result = json.dumps(envelope)
    return json_result


def deep_merge(new_dict, result_dict, list_method='replace'):
    '''
    perform a "deep" merge of two nested dictionaries. Currently this is
    intentionally restricted to dicts, hence the use of isinstance().
    '''

    for key in new_dict:
        if key in result_dict:

            if isinstance(new_dict[key], dict) and \
            isinstance(result_dict[key], dict):

                # recurse
                deep_merge(new_dict[key], result_dict[key])


            elif isinstance(result_dict[key], dict) and not \
            isinstance(new_dict[key], dict):

                raise Exception('cannot merge, incompatible types')


            elif isinstance(new_dict[key], list) and \
            isinstance(result_dict[key], list) and \
            list_method == 'add':

                # don't recurse into lists for now, just combine

                for entry in new_dict[key]:
                    if entry not in result_dict[key]:

                        rlogger.debug('adding list item {item}'.format(
                            item=entry))

                        result_dict[key].append(entry)

            else:
                result_dict[key] = new_dict[key]

        else:

            # add the new value to the result
            result_dict[key] = new_dict[key]


def catch_exception(func):

    def _func_wrapper(*args, **kwargs):

        try:
            result = func(*args, **kwargs)
            # rlogger.status('Ready')

            return result


        except:
            msg = 'ERROR - caught unhandled exception in function {func} at {module}'
            msg = msg.format(func=func.__name__, module=func.__module__)
            rlogger.error(msg)
            # rlogger.status(msg)

            for line in traceback.format_exc().split('\n'):
                rlogger.error(line)

    return _func_wrapper


def slugify(value, separator='_'):
    '''
    NOTE: this was lifted from django.
    Slugify a string, to make it URL friendly.
    '''
    import unicodedata

    # force unicode since the following code requires it
    value = '{value}'.format(value=value)

    value = unicodedata.normalize('NFKD', value).encode('ascii', 'ignore')
    value = re.sub('[^\w\s-]', '', value.decode('ascii')).strip().lower()
    return re.sub('[%s\s]+' % separator, separator, value)


def create_desktop_shortcut(name, exec_path, icon_path, working_dir=None):

    if os.name != 'nt':
        return False

    import pythoncom
    from win32com.shell import shell, shellcon

    shortcut = pythoncom.CoCreateInstance (
        shell.CLSID_ShellLink,
        None,
        pythoncom.CLSCTX_INPROC_SERVER,
        shell.IID_IShellLink
    )

    if not working_dir:
        working_dir = os.path.dirname(exec_path)

    shortcut.SetPath(exec_path)
    shortcut.SetDescription(name)
    shortcut.SetIconLocation(icon_path, 0)
    shortcut.SetWorkingDirectory(working_dir)

    lnk_name = '{}.lnk'.format(name)

    desktop_path = shell.SHGetFolderPath(0, shellcon.CSIDL_DESKTOP, 0, 0)
    shortcut_path = os.path.join(desktop_path, lnk_name)

    if os.path.isfile(shortcut_path):
        print('Warning: found existing shortcut, attempting remove: {}'.format(shortcut_path))
        os.remove(shortcut_path)

    persist_file = shortcut.QueryInterface(pythoncom.IID_IPersistFile)
    persist_file.Save(shortcut_path, 0)

    return True




def get_user():
    '''
    Get the currently logged in user, considering the current
    process is not the current logged in user. On Windows this
    is intended to work from a service that is not the currently
    logged in user
    '''

    def _get_user_win():

        import pythoncom
        pythoncom.CoInitialize()

        import wmi

        wmi_client = wmi.WMI()
        user_result = wmi_client.Win32_ComputerSystem()
        return os.path.basename(user_result[0].UserName)


    if os.name == 'nt':
        return _get_user_win()

    else:
        return os.getenv('USER', os.getenv('USERNAME'))