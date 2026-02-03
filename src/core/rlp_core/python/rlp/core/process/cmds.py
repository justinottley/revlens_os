
import os
import subprocess


def get_process_list(executable=None, arg_filter=None):
    
    def _get_process_list_win(exe=None):
        
        import wmi
        
        result = []
        wmi_client = wmi.WMI()
        
        for entry in wmi_client.Win32_Process(name=exe):
            
            entry_info = {
                'executable': entry.Name,
                'pid': entry.ProcessId,
                'args': entry.CommandLine
            }
            
            
            if arg_filter:
                if entry_info['args'] and arg_filter in entry_info['args']:
                    result.append(entry_info)
                    
            else:
                result.append(entry_info)
                
            
        return result
      
    if os.name == 'nt':
        return _get_process_list_win(executable)


def posix_killall(tag, skip_tags=None):

    if skip_tags is None:
        skip_tags = []

    # get all children
    proc_child_list = 'ps -ef | grep {}'.format(tag)

    pid_list = []
    r = subprocess.check_output(proc_child_list, shell=True).decode('utf-8')
    for line in r.split('\n'):
        if not line:
            continue

        if 'grep' in line:
            continue
        
        skip = False
        for skip_tag in skip_tags:
            if skip_tag in line:
                skip = True
                continue
        
        if skip:
            continue
        
        line_parts = line.split()
        proc_pid = line_parts[1]
        proc_cmd = ' '.join(line.split(':')[-1].split()[1:])

        pid_list.append((proc_pid, proc_cmd))


    for pid, pid_cmd in pid_list:
        print('  Killing {} - {}'.format(pid, pid_cmd))
        try:
            pass
            os.kill(int(pid), 9)
        except Exception as e:
            print('Error killing PID {} - {}'.format(pid, e))
