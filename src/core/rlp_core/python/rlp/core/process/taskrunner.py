
import os
import sys
import json
import uuid
import logging
import tempfile
import threading
import traceback

from rlp.core import environment

rlogger = logging.getLogger('rlp.{}'.format(__name__))

class TaskRunner(threading.Thread):
    '''
    Run an input function with input args and kwargs in a separate thread,
    serializing the basic completion state.
    Meant to be used as a server side utility, to instead use asyncrounous requests
    where the client periodically asks for progress and blocks (client side)
    instead of having the server block in the main thread running the request.
    The state is serialized to disk so that multiple processes can know
    what the basic completion state is (provided they have access to the state
    file)
    '''
    
    env = environment.make_env()
    SPOOL_DIR = '/'.join([tempfile.gettempdir(), env.user, 'tasks'])
    
    def __init__(self, func, args=None, kwargs=None):
        threading.Thread.__init__(self)
        
        
        self.func = func
        self.args = args or []
        self.kwargs = kwargs or {}
        
        self.result = None
        self.traceback = None
        
        self.state = 'not_started'
        self.run_id = str(uuid.uuid4())
        
        self.serialize()
        
        
    @property
    def state_file(self):
        
        if not os.path.isdir(self.SPOOL_DIR):
            os.makedirs(self.SPOOL_DIR)
            
        return '/'.join([self.SPOOL_DIR, self.run_id])
        
        
    @property
    def info(self):
        
        info = {'state':self.state,
                'func':self.func.__name__,
                'module':self.func.__module__,
                'traceback':self.traceback}
                
        for entry in [('result', self.result),
                      ('args', self.args),
                      ('kwargs', self.kwargs)]:
            
            name, val = entry
            try:
                serialized_val = json.dumps(val)
                info[name] = serialized_val
                
            except:
                pass
            
        
        return info
        
        
    def serialize(self):
        
        with open(self.state_file, 'w') as fh:
            fh.write(json.dumps(self.info, indent=4))
        
        
    def run(self):
        
        rlogger.info('{rid}: running "{func}" {args} {kwargs}'.format(
            rid=self.run_id,
            func=self.func.__name__,
            args=self.args,
            kwargs=self.kwargs))
        
        self.state = 'running'
        
        self.serialize()
        
        try:
            self.result = self.func(*self.args, **self.kwargs)
            
        except:
            
            exctype, value = sys.exc_info()[:2]
            
            rlogger.exception('error running task {rid} - got {et} {v}'.format(
                rid=self.run_id, et=exctype.__name__, v=value))
            
            self.traceback = traceback.format_exc()
            
            if exctype.__name__ == 'CalledProcessError':
                
                cmd_info = '  Returncode: {r}\n'.format(r=value.returncode)
                cmd_info += '  Cmd: {c}\n'.format(c=value.cmd)
                cmd_info += '  Output: {o}\n'.format(o=value.output)
                
                self.traceback += '\nCalledProcessError Info:\n' + cmd_info
            
            
        self.state = 'done'
        
        self.serialize()
        
        rlogger.debug('{rid}: done'.format(rid=self.run_id))
        
        
        
    @classmethod
    def get_info(cls, run_id):
        
        result = {'state':'unknown'}
        
        run_file = '/'.join([cls.SPOOL_DIR, run_id])
        if os.path.isfile(run_file):
            fh = open(run_file)
            try:
                result = json.load(fh)
                
            except:
                rlogger.error('could not read run file {rf}'.format(rf=run_file))
                rlogger.error(traceback.format_exc())
                
                
        return result
        

def task_progress(run_id):
    return json.dumps(TaskRunner.get_info(run_id))

