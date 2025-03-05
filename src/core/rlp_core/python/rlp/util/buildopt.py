'''
Provides a flexible platform string solution for multiflavour build systems,
including all the platform info relevant for building software in a
human readable format.

Additionally, runtime registration of arbitrary buildopts to be included
in the platform string is available. Things like compiler or build flavour
can be included in the platform string using buildopt registration.

Registration of buildopts is order-preserving, so that the buildopt values can be
changed to get a consistently named platform / buildopt string across multiple
builds. Variant parts are separated in the platform string by dashes by default.

Variants can be grouped, and buildopt groups become directories to achieve
subdirectory trees of buildopts, assuming you leave the group separator
as the os directory path separator.

An example base platform string for CentOS 6.5 might look like:

>>> get_platform()
'Linux-x86_64-CentOS_6.5'


inclusion of a "compiler" buildopt:

>>> register_buildopt('compiler', 'clang_5.1')
'Linux-x86_64-CentOS_6.5-clang_5.1'


changing the 'compiler' buildopt:

>>> register_buildopt('compiler', 'gcc_4.2')
'Linux-x86_64-CentOS_6.5-gcc_4.2'


create a new buildopt group:

>>> push_group()


set a compiler buildopt in the new group:

>>> register_buildopt('compiler', 'gcc_4.2')
'Linux-x86_64-CentOS_6.5/gcc_4.2'


change the compiler in the new buildopt group:

>>> register_buildopt('compiler', 'clang_5.1')
'Linux-x86_64-CentOS_6.5/clang_5.1'

'''

import os
import platform
import traceback
from collections import OrderedDict

_platform_manager = None

BUILDOPT_TYPE_PLATFORM = 'platform'
BUILDOPT_TYPE_OPTIONAL = 'optional'
BUILDOPT_TYPE_OPSYS_VERSION = 'opsys_version'

    
class ScaffoldVariantException(Exception): pass

class PlatformManager(object):
    '''
    handles the construction of a platform string.
    '''
    
    # A mapping for handling multple compatible platforms more easily
    #
    # Example: 
    # 'Linux-x86_64-CentOS_Linux_7.4.1708': 'Linux-x86_64-CentOS_Linux_7.3.1611'
    
    _PLATFORM_MAP = {}
    
    def __init__(self, sep='-', group_sep=os.path.sep):
        
        self._opsys_ver_major = False
        self._opsys_ver_minor = False
        self._opsys_ver_patch = False

        self._init_opsys_ver_defaults()

        self.buildopts = OrderedDict()
        
        self.__sep = sep
        self.__group_sep = group_sep
        
        plat_sys = platform.system()
        if not plat_sys:
            plat_sys = 'unknown'

        self.buildopts['opsys'] = {'name':plat_sys,
                                  'type':BUILDOPT_TYPE_PLATFORM}
                                  
        self.buildopts['arch'] = {'name':platform.machine(),
                                 'type':BUILDOPT_TYPE_PLATFORM}
        
        
        vc_ver_info = self._get_vc_version()
        if len(vc_ver_info) == 5:
            
            self.buildopts['vc_version.full'] = {
                'name': vc_ver_info[0],
                'type': BUILDOPT_TYPE_OPTIONAL
            }
            
            self.buildopts['vc_version.minor_full'] = {
                'name': vc_ver_info[1],
                'type': BUILDOPT_TYPE_OPTIONAL
            }
            
            
        
        self.group_idx_list = []
        
        try:
            ver_major, ver_minor, ver_patch = getattr(
                self, '_get_opsys_version_{base_platform}'.format(
                    base_platform=self.buildopts['opsys']['name'].lower()))()
                    
            self.buildopts['opsys_version.major'] = {
                'name': ver_major,
                'type':BUILDOPT_TYPE_OPSYS_VERSION
            }

            self.buildopts['opsys_version.minor'] = {
                'name': ver_minor,
                'type': BUILDOPT_TYPE_OPSYS_VERSION
            }
            
            self.buildopts['opsys_version.patch'] = {
                'name': ver_patch,
                'type': BUILDOPT_TYPE_OPSYS_VERSION
            }


        except Exception as e:
            raise ScaffoldVariantException(
                'unsupported base OS - "{base_os}" {exc} : {message}'.format(
                    base_os=self.buildopts['opsys']['name'],
                    exc=e.__class__.__name__, message=str(e)))

        try:
            import socket
            hostname = socket.gethostname()
        except:
            hostname = 'unknown'

        self.buildopts['hostname'] = {'name': hostname,
                                     'type':BUILDOPT_TYPE_OPTIONAL}
        
        
    def _init_opsys_ver_defaults(self):

        if platform.system() == 'Darwin':
            self._opsys_ver_major = True
            self._opsys_ver_minor = True

        elif os.name == 'nt':
            self._opsys_ver_major = True


    def _get_vc_version(self):
        
        path = __file__
        for x in range(6):
            path = os.path.dirname(path)
        
        vc_version = os.path.basename(path)
        
        result = []
        
        try:
            vc_version_parts = vc_version.split('.')
            if len(vc_version_parts) == 3:
                ver_major = vc_version_parts[0]
                ver_minor = vc_version_parts[1]
                ver_patch = vc_version_parts[2]
                
                ver_minor_full = '.'.join(vc_version_parts[0:2])
                
                result = [vc_version, ver_minor_full, ver_major, ver_minor, ver_patch]
            
        except:
            print(traceback.format_exc())
            
            
        return result
        
        
    def _get_opsys_version_linux(self):
    	
        if hasattr(platform, "linux_distribution"):
            (dist_name, opsys_version, dist_id) = platform.linux_distribution()
        else:
            plat_info = platform.uname()
            dist_name = plat_info.system
            opsys_version = plat_info.release
            dist_id = ""
            
        try:
            ver_parts = opsys_version.split('.')
            ver_major = ver_parts[0]
            ver_minor = ''
            ver_patch = ''

            if len(ver_parts) > 1:
                ver_minor = ver_parts[1]

            if len(ver_parts)> 2:
                ver_patch = ver_parts[2]

            ver_major = '{dn}_{ver}'.format(
                dn=dist_name.replace(' ', '_'),
                ver=ver_major
            )
        except:
            print('Warning: could not get major, minor, patch versions from opsys version {v}'.format(v=opsys_version))
            print(traceback.format_exc())

            ver_major = opsys_version

        
        return (ver_major, ver_minor, ver_patch)
        
        
    def _get_opsys_version_darwin(self):
        '''
        Mac OSX
        '''
        
        (opsys_version, _, _) = platform.mac_ver()
        try:
            ver_parts = opsys_version.split('.')
            ver_major = ver_parts[0]
            ver_minor = ver_parts[1]
            ver_patch = ''
            if len(ver_parts) > 2:
                ver_patch = ver_parts[2]

        except:
            print('Warning: could not get major, minor, patch versions from opsys version {v}'.format(v=opsys_version))

            ver_major = opsys_version
            ver_minor = ''
            ver_patch = ''

        return (ver_major, ver_minor, ver_patch)


    def _get_opsys_version_windows(self):
        
        opsys_version_info = platform.win32_ver()
        try:
            ver_major, ver_minor, ver_patch = opsys_version_info[1].split('.')
            if opsys_version_info[0] in ['post2008Server', '7']:
                print('scaffold.buildopt: WARNING: mapping Windows version "{osv}" -> "10"'.format(
                    osv=opsys_version))
                ver_major = '10'
            
        except:
            print('Warning: could not get major, minor, patch versions from opsys version {v}'.format(v=opsys_version))


        return (ver_major, ver_minor, ver_patch)

    def _get_opsys_version_emscripten(self):
        return (1, 0, 0)

    def _get_opsys_version_unknown(self):
        return (1, 0, 0)

    def push_group(self):
        self.group_idx_list.append(len(self.buildopts))
        
    
    
    def _get_platform_opsys_version(self, major=None, minor=None, patch=None):
        
        platform_parts = []
        if major or (os.getenv('SCAFFOLD_BUILDOPT_OPSYS_VER_MAJOR') == '1') or self._opsys_ver_major:
            platform_parts.append(self.buildopts['opsys_version.major']['name'])

        if minor or (os.getenv('SCAFFOLD_BUILDOPT_OPSYS_VER_MINOR') == '1') or self._opsys_ver_minor:
            platform_parts.append(self.buildopts['opsys_version.minor']['name'])

        if patch or (os.getenv('SCAFFOLD_BUILDOPT_OPSYS_VER_PATCH') == '1') or self._opsys_ver_patch:
            platform_parts.append(self.buildopts['opsys_version.patch']['name'])

        return '.'.join(platform_parts)

    
    def _get_platform_raw(self, major=None, minor=None, patch=None):
        
        group_idx_list = [len(self.buildopts)]
        if self.group_idx_list:
            group_idx_list = self.group_idx_list + group_idx_list
            
        
        curr_group_idx = 0
        group_items = []
        for group_idx in group_idx_list:
            
            group_items.append(
                self.__sep.join([buildopt['name'] for buildopt in \
                    list(self.buildopts.values())[curr_group_idx:group_idx] \
                if buildopt['type'] == BUILDOPT_TYPE_PLATFORM]))
            
            curr_group_idx = group_idx
            
        platform_str =  self.__group_sep.join(group_items)
        platform_opsys_ver = self._get_platform_opsys_version(major, minor, patch)
        platform_list = [platform_str]
        if platform_opsys_ver:
            platform_list.append(platform_opsys_ver)
        platform_result = '-'.join(platform_list)
        
        return platform_result
        

    
    def _get_platform_mapped(self, major=None, minor=None, patch=None):
        
        platform = self._get_platform_raw(major, minor, patch)
        if platform in self._PLATFORM_MAP:
            platform = self._PLATFORM_MAP[platform]
        
        return platform


    @property
    def platform(self):
        '''
        a full platform string including all platform buildopts, 
        with groups separated by the group separator
        '''
        return self._get_platform_mapped()


def register_buildopt(buildopt, value, buildopt_type=BUILDOPT_TYPE_PLATFORM):
    '''
    register a buildopt to be included in the platform string
    '''
    
    global _platform_manager
    assert(_platform_manager)
    
    _platform_manager.buildopts[buildopt] = {'name':value, 'type':buildopt_type}
    
    
def get_platform(major=None, minor=None, patch=None, mapped=True):
    '''
    Get a platform string, including all registered buildopts
    
    The platform may be mapped to a different value to homogenize compatible
    platforms. Use mapped=False to get the actual, non-mapped platform string
    '''
    
    global _platform_manager
    assert(_platform_manager)
    
    if 'platform_target' in _platform_manager.buildopts:
        return _platform_manager.buildopts['platform_target']['name']

    elif mapped:
        return _platform_manager._get_platform_mapped(major, minor, patch)
        
    else:
        return _platform_manager._get_platform_raw(major, minor, patch)
    

def get_platform_list():

    return [
        get_platform(major=True, minor=True, patch=True),
        get_platform(major=True, minor=True, patch=False),
        get_platform(major=True, minor=False, patch=False),
        get_platform(major=False, minor=False, patch=False)
    ]


def get_buildopt(buildopt, name=True):
    
    global _platform_manager
    assert(_platform_manager)
    
    if name:
        return _platform_manager.buildopts[buildopt]['name']
        
    return _platform_manager.buildopts[buildopt]
    
    
def get_buildopts():
    
    global _platform_manager
    assert(_platform_manager)
    
    return _platform_manager.buildopts.keys()
    
    
def push_group():
    
    global _platform_manager
    assert(_platform_manager)
    
    return _platform_manager.push_group()
    
    
def match(input_buildopts):
    '''
    For an input buildopt dict, determine whether all the mappings
    correpond to the current platform buildopts.
    example input : {'opsys':'Linux', 'arch':'x86_64'}
    '''
    
    global _platform_manager
    assert(_platform_manager)
    
    result = True
    
    
    for buildopt_name, buildopt_value in input_buildopts.items():
        if buildopt_name not in _platform_manager.buildopts or \
        buildopt_value != _platform_manager.buildopts[buildopt_name]['name']:
            
            result = False
            
            
    return result
    
    
def match_any(input_buildopt_list):
    '''
    If any of the input buildopts match, return True
    '''
    
    for entry in input_buildopt_list:
        if match(entry):
            return True
            
    return False
    
    
def match_result(input_buildopt_list):
    '''
    Take a list of input buildopts with a corresponding return result value,
    and return the input return value if a match is found against that group of
    input buildopts.
    
    example input: [{'buildopts':{'opsys':'Linux', 'arch':'x86_64'},
                     'result':'return result for Linux-x86_64'},
                    {'buildopts':{'opsys':'Windows', 'arch':'AMD32'},
                     'result':'return result for Windows-AMD32'}]
                     
    If the current platform is {opsys=Linux,arch=x86_64}, return value is
    "return result for Linux-x86_64"
    '''
    
    for input_buildopt_info in input_buildopt_list:
        if match(input_buildopt_info['buildopts']):
            return input_buildopt_info['result']
    
    
def get_python_version(patchlevel=False):
    '''
    Convenience function to return the current python version as a dot-delimited
    version string. The version format not including the patchlevel is the
    most common, for example used on the filesystem (on posix platforms anyway)
    as the parent for the site-packages directory.
    '''
    
    
    ver_tuple = platform.python_version_tuple()
    version = '.'.join(ver_tuple[0:2])
    
    if patchlevel:
        version = '.'.join(ver_tuple)
        
    return version
    
    
def get_python_dirname(*args, **kwargs):
    '''
    Convenience function to return a "python directory name", just the word
    "python" followed by the python version. This format is the common
    directory naming convention for housing python's standard library and
    site-packages.
    '''
    
    return 'python{version}'.format(version=get_python_version(*args, **kwargs))
    
    
def _init_manager():
    
    global _platform_manager
    _platform_manager = PlatformManager()
    
    
_init_manager()
del _init_manager
