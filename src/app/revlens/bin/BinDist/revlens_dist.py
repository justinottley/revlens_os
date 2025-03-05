import os
import sys
import json
import platform
import datetime
import argparse

import scaffold.variant as buildopt

revlens_root = '/home/justinottley/dev/revlens_root'
superproject_root = 'sp_revsix'


pyver_parts = platform.python_version_tuple()
PYVER = '{}.{}'.format(pyver_parts[0], pyver_parts[1])

if os.name == 'nt':
    revlens_root =  r'C:\Users\justi\dev\revlens_root'

if platform.system() == 'Darwin':
    revlens_root = '/Users/justinottley/dev/revlens_root'


def _get_platform_str():
    plat_str = 'Linux'
    if os.name == 'nt':
        plat_str = 'Windows'

    elif platform.system() == 'Darwin':
        plat_str = 'Darwin'

    return plat_str


def dist_thirdbase():

    # rsync_cmd    
    pass

def path_format(win_path):
    return win_path.replace('C:', '/c').replace('\\', '/')


def dist_revlens(dist_ver):

    # top_dir = os.path.dirname(__file__)
    # for x in range(5):
    #     top_dir = os.path.dirname(top_dir)

    platform_str = _get_platform_str()

    top_dir = revlens_root

    
    platform_reldir = os.path.join(top_dir, superproject_root, 'inst', platform_str)
    dist_toptopdir = os.path.join(top_dir, 'dist', 'revlens_{}'.format(dist_ver))
    dist_topdir = os.path.join(top_dir, 'dist', 'revlens_{}'.format(dist_ver), dist_ver)
    dist_revsix_topdir = os.path.join(dist_topdir, 'sp_revsix')

    dist_build_dir = os.path.join(top_dir, 'dist', 'revlens', dist_ver)
    dist_revsix_dir = os.path.join(dist_build_dir, 'sp_revsix')

    dist_reldir = os.path.join(dist_topdir, superproject_root, 'inst', platform_str)
    
    print('Disting to: {d}'.format(d=dist_reldir))
    
    if not os.path.isdir(dist_reldir):
        print('Creating {d}'.format(d=dist_reldir))
        os.makedirs(dist_reldir)
    
    if not os.path.isdir(dist_build_dir):
        print('Creating {}'.format(dist_build_dir))
        os.makedirs(dist_build_dir)

    if not os.path.isdir(dist_revsix_dir):
        print('Creating link to build dir: {}'.format(dist_revsix_dir))
        cmd = 'ln -s {} {}'.format(
            dist_revsix_topdir, os.path.dirname(dist_revsix_dir)
        )
        print(cmd)
        os.system(cmd)


    print(platform_reldir)
    print(dist_reldir)
    
    for sset_name in os.listdir(platform_reldir):

        global_sset_dir = os.path.join(platform_reldir, sset_name)
        dist_sset_dir = os.path.join(dist_reldir)
        if os.name == 'nt':
            dist_sset_dir = os.path.join(dist_reldir, sset_name)

        if not os.path.isdir(global_sset_dir):
            continue

        if not os.path.isdir(dist_sset_dir):
            print('Creating {d}'.format(d=dist_sset_dir))
            os.makedirs(dist_sset_dir)

        # print global_sset_dir
        # print dist_sset_dir
        
        copy_cmd = None
        if os.name == 'nt':
            copy_cmd = r'robocopy /MIR {} {}'.format(global_sset_dir, dist_sset_dir)

        else:
            copy_cmd = 'rsync -rvzL --progress {src} {dest}'.format(
                src=path_format(global_sset_dir),
                dest=path_format(dist_sset_dir)
            )

        print(copy_cmd)
        os.system(copy_cmd)

        if sset_name == 'sclib':
            print('FOUND SCLIB')
            print(global_sset_dir)
            
            # copy to build dir
            copy_cmd = 'rsync -rvzL --progress {} {}'.format(
                global_sset_dir,
                dist_build_dir
            )
            print(copy_cmd)
            os.system(copy_cmd)

        dirs_to_remove = []

        for root, dirs, files in os.walk(dist_sset_dir):
            if '__pycache__' in root:
                dirs_to_remove.append(root)

            if 'include' in root:
                dirs_to_remove.append(root)

            if '_gen' in root:
                dirs_to_remove.append(root)


        for dir_to_remove in dirs_to_remove:

            print('')
            print('-- RUNNING CLEAN --')
            print('')

            if os.name == 'nt':
                pycache_dir_clean_cmd = 'rmdir /s /q {}'.format(dir_to_remove)
            else:
                pycache_dir_clean_cmd = 'rm -rf {}'.format(dir_to_remove)

            print(pycache_dir_clean_cmd)
            os.system(pycache_dir_clean_cmd)
        

        # python byte compile
        import compileall
        compileall.compile_dir(dist_sset_dir, legacy=True)

        # py_byte_compile = 'python -c "import compileall;compileall.compile_dir(r\'{}\', )"'.format(dist_sset_dir)
        # print(py_byte_compile)
        # os.system(py_byte_compile)

        for root, dirs, files in os.walk(dist_sset_dir):
            for file_entry in files:

                # .exp - Windows exports
                # .lib - Windows compiled library

                if (file_entry.endswith('.py') and \
                file_entry != 'startup.py') and \
                file_entry != 'rlp_io_service.py' and \
                file_entry != 'rlp_py.py' or \
                file_entry.endswith('.exp') or \
                file_entry.endswith('.lib'):

                    file_path = os.path.join(root, file_entry)
                    print('cleaning {}'.format(file_path))

                    if os.name == 'nt':
                        clean_cmd = 'del {}'.format(file_path)
                    else:
                        clean_cmd = 'rm -f {}'.format(file_path)

                    print(clean_cmd)
                    os.system(clean_cmd)

        
        '''
        for root, dirs, files in os.walk(dist_sset_dir):
            for file_entry in files:
                print(root)
                if 'web2py' in root:
                    continue

                if file_entry.endswith('.py') and file_entry not in ['startup.py', 'anyserver.py']:
                    
                    file_path = os.path.join(root, file_entry)
                    print('Removing {f}'.format(f=file_path))
                    os.remove(file_path)
        '''

    # shuffle qt.conf
    #
    print('Setting up qt.conf')
    dist_bin_dir = os.path.join(dist_reldir, 'revlens', 'bin')

    if buildopt.get_variant('opsys') == 'Linux':
        os.rename(
            os.path.join(dist_bin_dir, 'qt.conf'),
            os.path.join(dist_bin_dir, 'qt.conf.dev'))

        os.rename(
            os.path.join(dist_bin_dir, 'qt.conf.dist'),
            os.path.join(dist_bin_dir, 'qt.conf'))

    
    revlens_exec_path = None
    if os.name == 'nt':
        # generate bat executable for windows
        print(dist_bin_dir)

        #
        # revlens executable
        #

        revlens_exec_path = os.path.join(dist_topdir, 'revlens_wlogin.bat')
        bat_text = 'set RLP_FS_REMOTE=wss://studio2.revlens.io:8003\n'
        bat_text += 'set RLP_FS_ROOT=R:\n\n'
        bat_text += r'%~dp0\..\thirdbase\22_09\Windows-AMD64-10\Python\{}\python.exe %~dp0\sp_revsix\inst\Windows\revlens\bin\revlens.pyc %*'.format(PYVER)
        
        with open(revlens_exec_path, 'w') as wfh:
            wfh.write(bat_text)

        #
        # launcher executable
        #

        exec_path = os.path.join(dist_topdir, 'rlp_launcher.bat')

        bat_text = 'set RLP_FS_REMOTE=ws/s://studio2.revlens.io:8003\n'
        bat_text += 'set RLP_FS_ROOT=R:\n\n'
        bat_text += r'%~dp0\..\thirdbase\22_09\Windows-AMD64-10\Python\{}\python.exe %~dp0\sp_revsix\inst\Windows\prod\bin\rlp_launcher.pyc %*'.format(PYVER)
        
        with open(exec_path, 'w') as wfh:
            wfh.write(bat_text)

        tr_bat_text = 'set RLP_FS_REMOTE=ws/s://studio2.revlens.io:8003\n'
        tr_bat_text += 'set RLP_FS_ROOT=R:\n\n'
        tr_bat_text += '\n@REM Disable login screen on launch for top-level executable\n'
        tr_bat_text += 'set RLP_NO_LOGIN=1\n\n'
        tr_bat_text += r'%~dp0\thirdbase\22_09\Windows-AMD64-10\python\{}\python.exe %~dp0\{}\sp_revsix\inst\{}\revlens\bin\revlens.pyc %*'.format(PYVER, dist_ver, _get_platform_str())

        revlens_top_exec_path = os.path.join(dist_toptopdir, 'revlens.bat')
        with open(revlens_top_exec_path, 'w') as wfh:
            wfh.write(tr_bat_text)


        # rename standalone python binding module....
        core_site_packages_dir = os.path.join(dist_reldir, 'core', 'lib', 'python{}'.format(PYVER), 'site-packages')
        src_pylib = os.path.join(core_site_packages_dir, 'RlpCorePYMODULE.dll')
        dest_pylib = os.path.join(core_site_packages_dir, 'libRlpCorePYMODULE.pyd')

        if not os.path.isfile(dest_pylib):
            os.rename(src_pylib, dest_pylib)
            print('{} -> {}'.format(src_pylib, dest_pylib))
        else:
            print('Skipping, file exists: {}'.format(dest_pylib))


    else:

        env_start_path = os.path.join(dist_topdir, superproject_root, 'inst', _get_platform_str(), 'revlens', 'bin', 'env_start.sh')
        env_start_text = '#!/bin/sh\n\n'
        env_start_text += 'export RR=../../../../..\n'
        env_start_text += 'export SR=$RR/sp_revsix/inst/{}\n'.format(_get_platform_str())

        if platform.system() == 'Darwin':
            env_start_text += 'export DYLD_LIBRARY_PATH=$SR/core/lib:$SR/gui/lib:$SR/edb/lib:$SR/revlens/lib:$SR/prod/lib:$TB/PythonQt/20.03_Qt_5.14.1_Py_3.7/lib:$TB/Qt/5.14.1/clang_64/lib\n'

        elif os.name == 'posix':
            env_start_text += 'export LD_LIBRARY_PATH=$SR/core/lib:$SR/gui/lib:$SR/edb/lib:$SR/revlens/lib:$SR/prod/lib:$TB/PythonQt/20.09_Qt_5.14.2_Py_3.8/lib:$TB/Qt/5.14.2/gcc_64/lib\n'

        
        # NOTE: not needed anymore
        # env_start_text += 'export REQUESTS_CA_BUNDLE=$RR/sp_revsix/inst/Linux-x86_64/web/etc/cert.pem\n'

        with open(env_start_path, 'w') as wfh:
            wfh.write(env_start_text)

        os.chmod(env_start_path, 0o777)


        revlens_exec_path = os.path.join(dist_topdir, 'revlens_wlogin')
        revlens_top_exec_path = os.path.join(dist_toptopdir, 'revlens')
        rlp_launcher_exec_path = os.path.join(dist_topdir, 'rlp_launcher')

        sh_text = '#!/usr/bin/env bash\n\n'
        sh_text += 'SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )\n\n'
        sh_text += '# NOTE: demo default, change this for site\n'
        # sh_text += 'export RLP_FS_REMOTE=wss://studio2.revlens.io:8003\n'
        sh_text += 'export RLP_FS_REMOTE=ws://127.0.0.1:8003\n'
        sh_text += 'export RLP_FS_ROOT=/tmp/fs\n\n'
        # sh_text += '. $SCRIPT_DIR/sp_revsix/inst/{}/revlens/bin/env_start.sh\n\n'.format(_get_platform_str())

        r_sh_text = 'export PATH=$SCRIPT_DIR/../thirdbase/22_09/Linux-x86_64/python' + PYVER + '/bin:$PATH\n'
        r_sh_text += 'export LD_LIBRARY_PATH=$SCRIPT_DIR/../thirdbase/22_09/Linux-x86_64/python/' + PYVER + '/lib\n'
        r_sh_text += 'export PYTHONHOME=$SCRIPT_DIR/../thirdbase/22_09/Linux-x86_64/python/' + PYVER + '\n'
        r_sh_text += '$SCRIPT_DIR/../thirdbase/22_09/Linux-x86_64/python/' + PYVER + '/bin/python3 $SCRIPT_DIR/sp_revsix/inst/{}/revlens/bin/revlens $@'.format(_get_platform_str())
        
        rl_sh_text = 'export PATH=$SCRIPT_DIR/../thirdbase/22_09/Linux-x86_64/python/' + PYVER + '/bin:$PATH\n'
        rl_sh_text += 'export LD_LIBRARY_PATH=$SCRIPT_DIR/../thirdbase/22_09/Linux-x86_64/python/' + PYVER + '/lib\n'
        rl_sh_text += 'export PYTHONHOME=$SCRIPT_DIR/../thirdbase/22_09/Linux-x86_64/python/' + PYVER+ '\n'
        rl_sh_text += '$SCRIPT_DIR/../thirdbase/22_09/Linux-x86_64/python/' + PYVER + '/bin/python3 $SCRIPT_DIR/sp_revsix/inst/{}/prod/bin/rlp_launcher $@'.format(_get_platform_str())

        tr_sh_text = '\n# Control login screen on launch\n'
        tr_sh_text += '# export RLP_NO_LOGIN=1\n\n'
        tr_sh_text += 'export PATH=$SCRIPT_DIR/thirdbase/22_09/Linux-x86_64/python/' + PYVER + '/bin:$PATH\n'
        tr_sh_text += 'export LD_LIBRARY_PATH=$SCRIPT_DIR/thirdbase/22_09/Linux-x86_64/python/' + PYVER + '/lib\n'
        tr_sh_text += 'export PYTHONHOME=$SCRIPT_DIR/thirdbase/22_09/Linux-x86_64/python/' + PYVER + '\n'
        tr_sh_text += '$SCRIPT_DIR/thirdbase/22_09/Linux-x86_64/python/' + PYVER + '/bin/python3 $SCRIPT_DIR/{}/sp_revsix/inst/{}/revlens/bin/revlens $@'.format(dist_ver, _get_platform_str())
        
        with open(revlens_exec_path, 'w') as wfh:
            wfh.write(sh_text)
            wfh.write(r_sh_text)

        with open(revlens_top_exec_path, 'w') as wfh:
            wfh.write(sh_text)
            wfh.write(tr_sh_text)

        with open(rlp_launcher_exec_path, 'w') as wfh:
            wfh.write(sh_text)
            wfh.write(rl_sh_text)

        os.chmod(revlens_exec_path, 0o777)
        os.chmod(revlens_top_exec_path, 0o777)
        os.chmod(rlp_launcher_exec_path, 0o777)
        

    assert(revlens_exec_path)
    print('Wrote {}'.format(revlens_exec_path))



def create_zip(dist_ver):

    dist_dir = os.path.join(revlens_root, 'dist')
    tb_src_dir = os.path.join(revlens_root, 'dist', 'thirdbase')
    ver_dir = os.path.join(revlens_root, 'dist', 'revlens_{}'.format(dist_ver))

    # tb_ver_dir = os.path.join(ver_dir, 'thirdbase')
    # if not os.path.isdir(tb_src_dir):
    #     print('WARNING: thirdbase source not found: {}'.format(tb_src_dir))

    # if os.path.isdir(tb_src_dir) and not os.path.isdir(tb_ver_dir):
    #     cmd = 'ln -s {} {}'.format(tb_src_dir, ver_dir)
    #     print(cmd)
    #     os.system(cmd)

    # print('Checking for {}'.format(tb_ver_dir))
    # assert(os.path.isdir(tb_ver_dir))

    plat_name = 'linux'
    if os.name == 'nt':
        plat_name = 'win64'
    
    else:
        if 'el9' in platform.release():
            plat_name = 'linux_RHEL9'
    
    
    zipcmd = 'cd {} && tar --xz -h -v -cf revlens_{}_{}_x86_64_debug_combined.tar.xz revlens_{}'.format(dist_dir, dist_ver, plat_name, dist_ver)
    print(zipcmd)
    os.system(zipcmd)


def main():

    parser = argparse.ArgumentParser('')
    parser.add_argument('--mode', dest='mode', default='dist', help='dist or zip')
    parser.add_argument('dist_ver', default=None, nargs='*')

    args = parser.parse_args()


    if args.dist_ver:
        dist_ver = sys.argv[1]
    else:
        platform_reldir = os.path.join(revlens_root, superproject_root, 'inst', _get_platform_str())
        build_time_file = os.path.join(platform_reldir, '.build_time')
        if not os.path.isfile(build_time_file):
            print('no build time file, aborting')
            print(build_time_file)
            sys.exit(1)

        build_epoch = open(build_time_file).read()
        now = datetime.datetime.fromtimestamp(int(build_epoch))

        print(now)
        dist_ver = '{y}.{m}.{d}'.format(y=str(now.year)[2:], m=str(now.month).zfill(2), d=str(now.day).zfill(2))


    print('Got DIST_VER: {}'.format(dist_ver))


    # os.environ['DIST_VER'] = dist_ver

    if args.mode == 'dist':
        dist_revlens(dist_ver)

    elif args.mode == 'zip':
        create_zip(dist_ver)

    print('Done')

if __name__ == '__main__':
    main()