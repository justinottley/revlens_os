import os
import sys
import subprocess


# QT_PATH = '/Users/justinottley/dev/revlens_root/thirdbase/180601/inst/Darwin-x86_64-10.13.5/Qt/5.9.5/clang_64/lib/Qt'
# QT_PATH = '@executable_path/../../../../../../thirdbase/180601/inst/Darwin-x86_64-10.13.5/Qt/5.9.5/clang_64/lib/Qt'
# QT_PATH = '@executable_path/../../../../../../thirdbase/180601/inst/Darwin-x86_64-10.13.5/Qt/5.41.1/clang_64/lib/Qt'

# QT_PATH = '@executable_path/../thirdbase/20_04/inst/Darwin-x86_64-10.16/Qt/5.14/clang_64/lib/Qt'
QT_PATH = '/Users/justinottley/dev/revlens_root/thirdbase/22_09/Darwin-arm64/Qt/6.8.2/macos/lib/Qt'
# QT_PATH = '../thirdbase/22_09/Darwin-x86_64-11.7/Qt/6.5.3/macos/lib/Qt'
RPATH_STR = '@rpath/Qt'

revlens_root = '/Users/justinottley/dev/revlens_root/sp_revsix/inst/Darwin-x86_64-11.7'
py_dirs = 'global/001/lib/python3.11/site-packages'
_executable_list = [
    '{}/core/_projects/core/{}/libRlpCorePYMODULE.so',
    '{}/edb/_projects/edb/{}/libRlpEdbDB_PYMODULE.so',
    '{}/edb/_projects/edb/{}/libRlpEdbFS_PYMODULE.so'
]

executable_list = []
for entry in _executable_list:
    executable_list.append(entry.format(revlens_root, py_dirs))


def run_fix(exec_in):

    if not os.path.isfile(exec_in):
        print('NOT FOUND: {}, skipping'.format(exec_in))
        return

    cmd = 'otool -L {path}'.format(path=exec_in)
    
    print(cmd)
    result = subprocess.check_output(cmd, shell=True)
    result = result.decode('utf-8')
    print(result)
    for line in result.split('\n'):
        line = line.strip()
        # print line
        if line.find(RPATH_STR) != -1:
            
            line_parts = line.split()
            lib_part = line_parts[0]
            new_line = lib_part.replace(RPATH_STR, QT_PATH)
            
            change_cmd = 'install_name_tool -change '
            change_cmd += lib_part + ' ' + new_line
            change_cmd += ' ' + exec_in
            
            print(change_cmd)
            os.system(change_cmd)
          

def main():
    
    executable = sys.argv[1]
    run_fix(executable)

    # for entry in executable_list:
    #     run_fix(entry)

      
    
if __name__ == '__main__':
    main()