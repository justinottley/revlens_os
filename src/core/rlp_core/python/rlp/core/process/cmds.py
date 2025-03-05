
import os

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
