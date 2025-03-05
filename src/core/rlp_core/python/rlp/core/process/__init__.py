
import os
import time
import signal
import logging
import threading
import subprocess

from rlp.core import rlp_logging

rlogger = logging.getLogger('rlp.{}'.format(__name__))


class ProcessEventLoop(object):
    '''
    An event loop interface for asyncronous listening
    '''
    
    def __init__(self, process):
        self.process = process
        
    def start(self):
        raise NotImplementedError
        
    def wait(self):
        raise NotImplementedError
        
        
class ThreadingEventLoop(ProcessEventLoop):
    
    use_proc_wait = False
    
    def __init__(self, process):
        ProcessEventLoop.__init__(self, process)
        
        self.thread = None
        
        
    def start(self):
        
        self.thread = threading.Thread(target=self._read_thread)
        self.thread.setDaemon(True)
        self.thread.start()
        
        
    def wait(self):
        
        if self.use_proc_wait and self.process.proc:
            rlogger.verbose('waiting for process to exit..')
            self.process.proc.wait()
            
        else:
            
            rlogger.verbose('waiting for listener thread to exit..')
            self.thread.join()
            
            
        # XXX: Disabled for now
        # Process.remove_process(self.process)
        
        
    def _read_thread(self):
        
        def process_line():
            
            line = self.process.proc.stdout.readline()
            for listener in self.process.listeners:
                try:
                    listener(line)
                    
                except Exception as e:
                    rlogger.warning('error running listener {listener} - {exc}: {message}'.format(
                        listener=listener, exc=e.__class__.__name__, message=str(e)))
                    
            return line
            
            
        while process_line():
            pass
        
        # turn off this logging line - causes weird problems with atexit handlers
        # rlogger.verbose('read thread exiting')
        
        
        
class ExitListener(object):
    
    def __init__(self, process):
        
        self.process = process
        
    def __call__(self, line):
        
        if not line:
            Process.remove_process(self.process)
        
        
class Process(object):
    
    _processes = []
    _lock = threading.Lock()
    
    def __init__(self, reactor=ThreadingEventLoop):
        
        self.proc = None
        self.lock = threading.Lock()
        self.reactor = reactor(self)
        
        self._listeners = []
        
        
    @property
    def listeners(self):
        
        self.lock.acquire()
        listeners = self._listeners[:]
        self.lock.release()
        
        return listeners
        
        
    @classmethod
    def get_processes(cls):
        
        cls._lock.acquire()
        process_list = cls._processes[:]
        cls._lock.release()
        
        return process_list
        
        
    @classmethod
    def add_process(cls, process):
        
        rlogger.verbose('registering process {pid}'.format(pid=process.proc.pid))
        
        cls._lock.acquire()
        cls._processes.append(process)
        cls._lock.release()
        
        
    @classmethod
    def remove_process(cls, process):
        
        rlogger.verbose('removing process {pid}'.format(pid=process.proc.pid))
        
        cls._lock.acquire()
        if process in cls._processes:
            cls._processes.remove(process)
            
        else:
            rlogger.verbose('WARNING: {process} not found in process registry'.format(
                process=process))
            
        cls._lock.release()
        
        
    def run(self, cmd, cwd=None, shell=False, preexec_fn=None, env=None):
        
        rlogger.debug('running {cmd}'.format(cmd=cmd))
        
        # XXX: Disabled for now
        # self.add_listener(ExitListener(self))
        # Process.add_process(self)
        
        self.proc = subprocess.Popen(cmd,
                                     stdin=subprocess.PIPE,
                                     stdout=subprocess.PIPE,
                                     stderr=subprocess.STDOUT,
                                     cwd=cwd,
                                     shell=shell,
                                     preexec_fn=preexec_fn,
                                     env=env)
        
        
        
        self.reactor.start()
        
        
        
    def add_listener(self, listener):
        
        self.lock.acquire()
        self._listeners.append(listener)
        self.lock.release()
        
        
    def remove_listener(self, listener):
        
        self.lock.acquire()
        try:
            self._listeners.remove(listener)
            
        except Exception as e:
            
            rlogger.error('could not remove listener {listener} - {exc}: {message}'.format(
                listener=listener, exc=e.__class__.__name__, message=str(e)))
            
        finally:
            self.lock.release()
            
            
    def wait(self, timeout=None):
        
        result = None
        
        if timeout is None:
            result = self.proc.wait()
            
        elif timeout == 0:
            result = self.proc.poll()
            
        else:
            result = self.proc.poll()
            curr_time = time.time()
            timeout_time = curr_time + timeout
            
            while result is None and (curr_time < timeout_time):
                
                time.sleep(min(timeout_time - curr_time, 0.1))
                result = self.proc.poll()
                curr_time = time.time()
                
        if result is not None:
            # rlogger.verbose('proc wait result: {res} - starting reactor wait'.format(res=result))
            self.reactor.wait()
            
        return result
