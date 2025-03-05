'''
This is basically a hack to setup the environment. Yes it is quite .
It is basically a static hardcode of what sc_config and the scaffold
runtime user environment is supposed to do. This allows running without
the scaffold runtime user environment system
'''

import os
import sys
import pprint
import argparse
import platform


THIRDBASE_VERSION = '22_09'

PY_VERSION = '{}.{}'.format(
    sys.version_info.major,
    sys.version_info.minor
)

PY_VERSION_MINOR = '{}.{}.{}'.format(
    sys.version_info.major,
    sys.version_info.minor,
    sys.version_info.micro
)

PY_DIR = 'python{}'.format(PY_VERSION)


LD_LIB_PATH_KEY = None

INST_DIR = os.path.dirname(os.path.abspath(__file__))
for _ in range(6):
    INST_DIR = os.path.dirname(INST_DIR)

RUN_TOP_DIR = INST_DIR
for _ in range(2):
    RUN_TOP_DIR = os.path.dirname(RUN_TOP_DIR)

print('--')
print('Inst Dir: {idir}'.format(idir=INST_DIR))
print('Run Top Dir: {rdir}'.format(rdir=RUN_TOP_DIR))
print('--')

from . import buildopt

def _init_ldlib_path_key():

    global LD_LIB_PATH_KEY

    _ld_lib_path_key = 'DYLD_LIBRARY_PATH'
    if buildopt.get_buildopt('opsys') == 'Linux':
        _ld_lib_path_key = 'LD_LIBRARY_PATH'
        
    elif buildopt.get_buildopt('opsys') == 'Windows':
        _ld_lib_path_key = 'PATH'
    
    LD_LIB_PATH_KEY = _ld_lib_path_key


def setup_env_ffmpeg(env, tb_dir):

    ffmpeg_version = '7.1'
    if os.name == 'nt':
        ffmpeg_version = '4.4.1'

    if platform.system() == 'Darwin':
        ffmpeg_version = '4.4'

    ff_lib_dir = os.path.join(tb_dir, 'FFMpeg', ffmpeg_version, 'lib')
    ff_bin_dir = os.path.join(tb_dir, 'FFMpeg', ffmpeg_version, 'bin')

    for ff_lib in [ff_lib_dir, ff_bin_dir]: # , x264_lib_dir, png_lib_dir]:
        env[LD_LIB_PATH_KEY].insert(0, ff_lib)

    env['PATH'].append(ff_bin_dir)

def setup_env_openexr(env, tb_dir):

    openexr_version = '3.2.1'

    openexr_bin_dir = os.path.join(tb_dir, 'OpenEXR', openexr_version, 'bin')
    env[LD_LIB_PATH_KEY].insert(0, openexr_bin_dir)

    for lib_entry in ['lib64', 'lib']:
        openexr_lib_dir = os.path.join(tb_dir, 'OpenEXR', openexr_version, lib_entry)
        if os.path.isdir(openexr_lib_dir):
            env[LD_LIB_PATH_KEY].insert(0, openexr_lib_dir)

def setup_env_ocio(env, tb_dir):

    if os.name != 'nt':
        for project in [
            ('libGLEW', '2.1.0'),
            ('OpenColorIO', '2.3.0')
        ]:
            for lib_dir in ['lib64', 'lib']:
                
                project_name, project_version = project
                proj_lib_dir = os.path.join(tb_dir, project_name, project_version, lib_dir)
                if os.path.isdir(proj_lib_dir):
                    env[LD_LIB_PATH_KEY].insert(0, proj_lib_dir)
                    env['PYTHONPATH'].append(os.path.join(proj_lib_dir, 'site-packages'))

                else:
                    # print('NOT FOUND: {}'.format(proj_lib_dir))
                    pass


    if not os.getenv('OCIO'):
        print('WARNING: OCIO env. variable not set, using built-in default')

        # config = 'cg-config-v2.1.0_aces-v1.3_ocio-v2.3.ocio'
        # config = 'studio-config-v1.0.0_aces-v1.3_ocio-v2.1.ocio'
        config = 'studio-config-v2.1.0_aces-v1.3_ocio-v2.3.ocio'

        if os.name == 'nt':
            config = 'cg-config-v1.0.0_aces-v1.3_ocio-v2.1.ocio'

        ocio_config_path = os.path.join(tb_dir, 'OpenColorIO', '2.3.0', 'config', config)
        os.environ['OCIO'] = ocio_config_path.replace('\\', '/')


def setup_env_oiio(env, tb_dir):

    for project in [
        ('libTIFF', '4.6.0'),
        ('libjpeg-turbo', '3.0.1'),
        ('boost', '1.83.0'),
        ('OpenImageIO', '2.5.5.0')
    ]:
        for lib_dir in ['lib64', 'lib']:
            
            project_name, project_version = project
            proj_lib_dir = os.path.join(tb_dir, project_name, project_version, lib_dir)
            if os.path.isdir(proj_lib_dir):
                env[LD_LIB_PATH_KEY].insert(0, proj_lib_dir)
                pydir = os.path.join(proj_lib_dir, 'site-packages')
                if os.path.isdir(pydir):
                    env['PYTHONPATH'].append(pydir)

            else:
                # print('NOT FOUND: {}'.format(proj_lib_dir))
                pass


        bin_dir = os.path.join(tb_dir, project_name, project_version, 'bin')
        env['PATH'].append(bin_dir)


def setup_env_vcpkg(env, tb_dir):

    vcpkg_debug_dir = os.path.join(tb_dir, 'vcpkg', 'vcpkg', 'installed', 'x64-windows', 'debug', 'bin')
    # vcpkg_rel_dir = os.path.join(tb_dir, 'vcpkg', 'vcpkg', 'installed', 'x64-windows', 'bin')
    
    env['PATH'].append(vcpkg_debug_dir)
    # env['PATH'].append(vcpkg_rel_dir)


def setup_env_mypaint(env, tb_dir):

    mypaint_lib_dir = os.path.join(tb_dir, 'libmypaint', '1.0.0', 'lib')
    env[LD_LIB_PATH_KEY].insert(0, mypaint_lib_dir)


def setup_egg_pylibs(env, pydir):

    pydir_contents = os.listdir(pydir)
    pth_entries = []
    for file_entry in pydir_contents:
        if file_entry.endswith('.pth'):
            pth_entries.append(file_entry)

    for pth_entry in pth_entries:
        pth_path = os.path.join(pydir, pth_entry)
        print('Reading .pth: {}'.format(pth_path))
        with open(pth_path) as fh:
            for line in fh.readlines():
                if line.startswith('#'):
                    continue
                egg_lib = os.path.join(pydir, line.strip())

                # print('    {el}'.format(el=egg_lib))
                env['PYTHONPATH'].append(egg_lib)


def setup_env_platform(env, tb_dir):

    if os.name == 'nt':
        env['PATH'].append(os.path.join(tb_dir, 'DLLs', '001', 'debug'))
        # env['PATH'].append(os.path.join(tb_dir, 'OpenSSL-Win64', 'bin'))

    if platform.system() == 'Darwin':
        env[LD_LIB_PATH_KEY].insert(0, os.path.join(tb_dir, 'dylib_extra', '001', 'lib'))


def setup_env_scaffold(env, tb_dir):

    # dev path..
    scaffold_root_dir = os.path.join(
        os.path.expanduser('~'),
        'dev', 'revlens_root', 'release', buildopt.get_platform(), 'scaffold_root', '1.0'
    )

    if not os.path.isdir(scaffold_root_dir):

        scaffold_root_dir = os.path.join(
            tb_dir, 'scaffold_root', '1.0'
        )


    pylib_dir = os.path.join(scaffold_root_dir, 'lib', PY_DIR, 'site-packages')
    tmap_config_dir = os.path.join(pylib_dir, 'scaffold', 'ext', 'sclib', 'translation_config')

    tmap_path = [
        os.path.join(tmap_config_dir, 'local'),
        os.path.join(tmap_config_dir, 'network')
    ]

    env['PYTHONPATH'].append(pylib_dir)

    os.environ['SC_TRANSLATION_MAP_PATH'] = os.pathsep.join(tmap_path)
    os.environ['SC_PLATFORM'] = buildopt.get_platform()
    os.environ['SC_THIRDBASE_VERSION'] = '22_09'

    env['SC_TRANSLATION_MAP_PATH'] = tmap_path
    env['PATH'].append(os.path.join(scaffold_root_dir, 'bin'))


def setup_env_thirdparty(env, tb_dir):

    lib_list = [
        ('pylibs/ws4py', '0.5.1'),
        ('pylibs/requests', '2.31.0'),
        ('pylibs/shotgun_api3', '3.5.1'),
        ('pylibs/werkzeug', '0.0'),
        ('pylibs/watchdog', '3.0.0')
    ]
    
    if os.name == 'nt':
        lib_list += [
            ('pylibs/pywin32', '306')
        ]


    plat_name = platform.system()
    if plat_name == 'Linux':
        distro_name = 'Ubuntu' # FIXME: deprecated in python 3.8+ platform.linux_distribution()[0]
        if distro_name == 'LinuxMint':
            lib_list.append(
                ('OpenSSL', '1.0.2o/Mint')
            )

    elif plat_name == 'Windows':
        lib_list.append(
            ('WinFsp', '2022')
        )

    for lib_entry in lib_list:

        lib_name, lib_ver = lib_entry
        lib_dir = os.path.join(tb_dir, lib_name, lib_ver, 'lib')
        bin_dir = os.path.join(tb_dir, lib_name, lib_ver, 'bin')
        pylib_dir = os.path.join(lib_dir, PY_DIR, 'site-packages')

        env[LD_LIB_PATH_KEY].insert(0, lib_dir)
        if os.path.isdir(pylib_dir):

            env['PYTHONPATH'].append(pylib_dir)
            setup_egg_pylibs(env, pylib_dir)

        else:
            print('WARNING: skipping {}'.format(pylib_dir))

        if os.name == 'nt' and os.path.isdir(bin_dir):
            env['PATH'].append(bin_dir)


def setup_env_python(env, tb_dir):

    py_install_dir = os.path.join(tb_dir, 'python', PY_VERSION)
    py_lib_dir = os.path.join(py_install_dir, 'lib')
    
    if os.name == 'nt':
        os.environ['PYTHONHOME'] = py_install_dir
        py_lib_dir = py_install_dir

    elif platform.system() == 'Darwin':

        # Relocatable python:
        # https://github.com/gregneagle/relocatable-python
        #
        py_install_dir = os.path.join(tb_dir, 'Python.framework', 'Versions', PY_VERSION)
        py_lib_dir = os.path.join(py_install_dir, 'lib')

    env[LD_LIB_PATH_KEY].insert(0, py_lib_dir)

    os.environ['PYTHONUNBUFFERED'] = 'x'


def setup_env_qt(env, tb_path):

    qt_version = '6.8.2'
    plat_str = platform.system()
    if plat_str == 'Windows':
        qt_version = '6.4.2'

    qt_compiler = 'macos'
    qt_lib_dir = 'lib'
    qt_webengine_dir = 'libexec'

    if os.name == 'nt':
        qt_compiler = 'msvc2019_64'
        qt_lib_dir = 'bin'
        qt_webengine_dir = 'bin'
    
    elif buildopt.get_buildopt('opsys') == 'Linux':
        qt_compiler = 'gcc_64'

    qt_dir_root = os.path.join(tb_path, 'Qt', qt_version, qt_compiler)
    qt_dir_lib = os.path.join(qt_dir_root, qt_lib_dir)

    qnp_dir_lib = os.path.join(tb_path, 'qnanopainter', qt_version, qt_compiler, 'lib')

    # LD LIB PATH
    #
    env[LD_LIB_PATH_KEY].insert(0, qt_dir_lib)
    env[LD_LIB_PATH_KEY].insert(0, qnp_dir_lib)

    # LD_PRELOAD for CentOS
    #
    if platform.system() == 'Linux':
        
        linux_dist_name = 'Ubuntu' # platform.linux_distribution()[0]
        if 'CentOS' in linux_dist_name:

            preload_list = []
            for lib_entry in os.listdir(qt_dir_lib):
                if lib_entry.endswith('.so.5'):
                    preload_list.append(os.path.join(qt_dir_lib, lib_entry))

            print('LD_PRELOAD: {n} libraries'.format(n=len(preload_list)))
            os.environ['LD_PRELOAD'] = ':'.join(preload_list)

                


    # NOTE
    # qt.conf published with paths for Qt.
    # also QtWebEngineProcess has its own qt.conf, with Prefix=.. which seems to be the right
    # magic to get relocatable qt install and webengine happy
    #

    os.environ['QT_PLUGIN_PATH'] = os.path.join(qt_dir_root, 'plugins')
    # os.environ['QTWEBENGINEPROCESS_PATH'] = os.path.join(qt_dir_root, qt_webengine_dir, 'QtWebEngineProcess')
    
    # print(os.environ['QT_PLUGIN_PATH'])
    # print(os.environ['QTWEBENGINEPROCESS_PATH'])

    # for QtWebEngine / Chromium debugging
    #
    os.environ['QTWEBENGINE_REMOTE_DEBUGGING'] = '8890'


def setup_env_pyside(env, tb_dir):

    pyside_lib_dir = os.path.join(tb_dir, 'PySide6', '6.8.0_py312', 'lib')

    env[LD_LIB_PATH_KEY].append(pyside_lib_dir)
    env['PYTHONPATH'].append(os.path.join(pyside_lib_dir, PY_DIR, 'site-packages'))


def setup_env_rlp(env, rlp_name):

    rlp_core_sftset_dir = os.path.join(INST_DIR, rlp_name)

    rlp_core_lib_path = os.path.join(rlp_core_sftset_dir, 'lib')
    rlp_core_bin_path = os.path.join(rlp_core_sftset_dir, 'bin')

    env[LD_LIB_PATH_KEY].insert(0, rlp_core_lib_path)
    env['PYTHONPATH'].insert(0, os.path.join(rlp_core_lib_path, PY_DIR, 'site-packages'))
    env['PATH'].insert(0, rlp_core_bin_path)


def setup_env_revlens(env):

    for rlp_sftset_name in ['revlens', 'prod']:
        revlens_sftset_dir = os.path.join(INST_DIR, rlp_sftset_name)
        revlens_lib_dir = os.path.join(revlens_sftset_dir, 'lib')
        revlens_bin_dir = os.path.join(revlens_sftset_dir, 'bin')

        env[LD_LIB_PATH_KEY].insert(0, revlens_bin_dir)
        env[LD_LIB_PATH_KEY].insert(0, revlens_lib_dir)
        env['PYTHONPATH'].insert(0, os.path.join(revlens_lib_dir, PY_DIR, 'site-packages'))

        if LD_LIB_PATH_KEY != 'PATH':
            env['PATH'].insert(0, revlens_bin_dir)

    os.environ['REVLENS_PATH'] = os.getenv('REVLENS_PATH', '') + os.pathsep + os.path.join(INST_DIR, 'revlens')


def setup_env_extrevlens(env, ext_name):

    if not os.getenv('REVLENS_PATH'):
        print('WARNING: REVLENS_PATH not found')
        os.environ['REVLENS_PATH'] = ''
    
    extrevlens_dir = os.path.join(INST_DIR, ext_name)
    if os.path.isdir(extrevlens_dir):
        
        extrevlens_lib_dir = os.path.join(extrevlens_dir, 'lib')
        extrevlens_py_dir = os.path.join(extrevlens_lib_dir, PY_DIR, 'site-packages')
        os.environ['REVLENS_PATH'] = os.getenv('REVLENS_PATH', '') + os.pathsep + extrevlens_dir
        env['PYTHONPATH'].insert(0, extrevlens_py_dir)

        env[LD_LIB_PATH_KEY].insert(0, extrevlens_lib_dir)
        if os.path.isdir(extrevlens_py_dir):
            for revlens_plug in os.listdir(extrevlens_py_dir):
                revlens_plug_dir = os.path.join(extrevlens_py_dir, revlens_plug)
                if os.path.isdir(revlens_plug_dir):
                    env[LD_LIB_PATH_KEY].insert(0, revlens_plug_dir)

        else:
            print('WARNING: extrevlens python plugin dir not found: {}'.format(extrevlens_py_dir))


def setup_env(update_sys_path=False, full=True):

    global_dir = os.path.join(INST_DIR, 'prod')

    if '--service' in sys.argv:
        # Dont setup the environment again since service process
        # inherits environment (actually crashes on Windows?)
        return global_dir


    global buildopt
    _init_ldlib_path_key()

    # print('Revlens')
    # print('')

    platform = buildopt.get_platform()

    tb_dir = None
    if os.getenv('SC_ENV_INIT'):
        from scaffold.ext.path import Path
        sc_tb_dir = Path('thirdbase://').format(translation_map='local')
        if sc_tb_dir and os.path.isdir(sc_tb_dir):
            tb_dir = sc_tb_dir
            print('Got Thirdbase Dir from URI: {}'.format(tb_dir))

    if not tb_dir:
        tb_dir = os.path.join(RUN_TOP_DIR, 'thirdbase', THIRDBASE_VERSION, platform) #, 'scaffoldscaffold.vc.get_software_set('thirdbase').path
        if not os.path.isdir(tb_dir):
            print('Thirdbase dir not found, attempting parent: {}'.format(tb_dir))
            tb_dir = os.path.join(os.path.dirname(RUN_TOP_DIR), 'thirdbase', THIRDBASE_VERSION, platform)
            if not os.path.isdir(tb_dir):
                print('Thirdbase dir not found, attempting parent: {}'.format(tb_dir))
                tb_dir = os.path.join(os.path.dirname(os.path.dirname(RUN_TOP_DIR)), 'thirdbase', THIRDBASE_VERSION, platform)

    print('Thirdbase: {}'.format(tb_dir))
    print('')

    ldlib_path_key = LD_LIB_PATH_KEY
    env = {
        ldlib_path_key: [],
        'PYTHONPATH': []
    }
    
    if ldlib_path_key != 'PATH':
        env['PATH'] = []

    setup_env_platform(env, tb_dir)
    setup_env_python(env, tb_dir)
    setup_env_scaffold(env, tb_dir)
    setup_env_thirdparty(env, tb_dir)
    setup_env_qt(env, tb_dir)
    setup_env_pyside(env, tb_dir)

    setup_env_ffmpeg(env, tb_dir)
    setup_env_mypaint(env, tb_dir)
    setup_env_ocio(env, tb_dir)

    if os.name == 'nt':
        setup_env_vcpkg(env, tb_dir)
    else:
        setup_env_openexr(env, tb_dir)
        setup_env_oiio(env, tb_dir)


    setup_env_rlp(env, 'core')
    setup_env_rlp(env, 'gui')
    setup_env_rlp(env, 'edb')

    if full:
        setup_env_revlens(env)
        setup_env_extrevlens(env, 'prod')
        setup_env_extrevlens(env, 'extrevlens')
    else:
        setup_env_rlp(env, 'prod')

    # Flatten env
    if ldlib_path_key != 'PATH':
        os.environ[ldlib_path_key] = os.pathsep.join(env[ldlib_path_key])

    # win_path = [
    #     'C:\\WINDOWS\\system32',
    #     'C:\\WINDOWS',
    #     'C:\\WINDOWS\\System32\\Wbem',
    #     'C:\\WINDOWS\\System32\\WindowsPowerShell\\v1.0'
    # ]

    os.environ['PATH'] = os.pathsep.join(env['PATH']) + os.pathsep + os.getenv('PATH')
    os.environ['PYTHONPATH'] = os.pathsep.join(env['PYTHONPATH']) + os.pathsep + os.environ.get('PYTHONPATH', '')

    os.environ['REVLENS_APP_ROOT'] = INST_DIR
    os.environ['REVLENS_PLATFORM'] = platform
    os.environ['THIRDBASE_ROOT'] = tb_dir

    # os.environ['QSG_RENDER_LOOP'] = 'basic' # single threaded

    print(ldlib_path_key)
    for entry in os.environ[ldlib_path_key].split(os.pathsep):
        print('    {e}'.format(e=entry))


    if ldlib_path_key != 'PATH':
        print('PATH')
        for entry in os.environ['PATH'].split(os.pathsep):
            print('    {e}'.format(e=entry))
        
    print('')
    print('PYTHONHOME')
    print('    {}'.format(os.getenv('PYTHONHOME')))
    print('')
    print('')

    print('PYTHONPATH')

    for entry in os.environ['PYTHONPATH'].split(os.pathsep):
        print('    {}'.format(entry))

    if full:
        print('')
        print('REVLENS_PATH')

        for entry in os.environ['REVLENS_PATH'].split(os.pathsep):
            print('    {e}'.format(e=entry))
        
        print('')
        print('OCIO')
        print('    {}'.format(os.getenv('OCIO')))
        print('')

    if update_sys_path:
        sys.path = env['PYTHONPATH'] + sys.path
        
    # print('SYS.PATH')
    # for entry in sys.path:
    #     print('    {e}'.format(e=entry))
        
    return global_dir
