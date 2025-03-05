
import sys
import uuid
import time
import logging
import threading
import traceback
import collections


import rlp.core.util
from rlp.core import rlp_logging


class WorkerThread(threading.Thread):

    def __init__(self, queue, worker_kwargs):
        threading.Thread.__init__(self)

        self.LOG = logging.getLogger('rlp.{}.{}'.format(
            self.__class__.__module__, self.__class__.__name__))

        self.queue = queue
        self.dispatch_condition = queue.dispatch_condition
        self._shutdown = queue._shutdown

        self.worker_kwargs = worker_kwargs


    def init(self):
        pass

    def dispatch(self, run_id, payload):
        pass


    def run(self):
        
        self.init()

        while not self._shutdown.is_set():

            self.LOG.debug('waiting..')

            payload = None
            run_id = None

            with self.dispatch_condition:
                self.dispatch_condition.wait()

                run_id, payload = self.queue.pop_next_task()

            self.LOG.debug('Got task payload, dispatching')

            self.dispatch(run_id, payload)

            self.queue.finalize(run_id)




class IOQueue(object):

    def __init__(self, name, shutdown_event, worker_ns, worker_kwargs, size=1):

        self.LOG = logging.getLogger('rlp.{}.{}'.format(self.__class__.__module__, self.__class__.__name__))

        self.name = name

        self.lock = threading.Lock()

        self._shutdown = shutdown_event
        self.queue = collections.deque()
        self.task_dict = {}
        self.dispatch_condition = threading.Condition()

        self.worker_cls = rlp.core.util.import_function(worker_ns)

        self.workers = []
        for _ in range(size):

            self.LOG.debug('creating worker thread: {}'.format(self.worker_cls))

            worker_thread = self.worker_cls(self, worker_kwargs)
            worker_thread.start()

            self.workers.append(worker_thread)


    def pop_next_task(self):

        run_id, payload = self.queue.popleft()
        
        self.LOG.debug('pop next task: {}'.format(run_id))


        with self.lock:
            self.task_dict[run_id]['state'] = 'running'

        return (run_id, payload)


    def finalize(self, run_id):
        
        self.LOG.info('Finalizing: {}'.format(run_id))

        with self.lock:
            self.task_dict[run_id]['state'] = 'done'

    
    def has_pending_tasks(self):
        return len(self.queue) > 0

    def get_queue_size(self):
        # return the result by logging
        self.LOG.info('mreturn', extra={'extra': {
            'method': 'get_queue_size',
            'result': len(self.queue)}
        })

    def get_task_state(self, run_id):
        
        with self.lock:
            if run_id in self.task_dict:
                return self.task_dict[run_id]['state']


    def append(self, run_id, payload):

        msg = 'append {} {}'.format(run_id, payload['kwargs'])

        progress_info = {
            'value': 0,
            'action': payload['action'],
            'progress_str': 'queued',
            'store_relpath': '',
            'metadata': {'run_id': run_id},
        }

        if 'fs_path' in payload['kwargs']:
            progress_info['path'] = payload['kwargs']['fs_path']
            progress_info['display_title'] = payload['kwargs']['fs_path']


        self.LOG.progress(msg, extra={'extra':progress_info})

        with self.lock:
            self.task_dict[run_id] = {
                'state': 'queued',
                'payload': payload
            }
        
        
        self.queue.append((run_id, payload))



class IOScheduler(object):

    def __init__(self):

        self.LOG = logging.getLogger('rlp.{}.{}'.format(self.__class__.__module__, self.__class__.__name__))

        self._shutdown = threading.Event()
        self._run = threading.Event()

        self.queues = {}


    def register_queue(self, size=1, **kwargs):

        print('registering queue - "{}" {}'.format(kwargs.get('qname'), kwargs.get('worker_cls')))

        name = kwargs['qname']
        worker_ns = kwargs['worker_cls']
        worker_kwargs = kwargs['worker_info']

        try:
            ioqueue = IOQueue(
                name,
                self._shutdown,
                worker_ns,
                worker_kwargs,
                size=size
            )

            self.queues[name] = ioqueue

            self.LOG.info('queue ready', extra={'extra': {
                'method': 'queue_ready',
                'result': {
                    'queue_name': name
                }
            }})



        except:
            self.LOG.error(traceback.format_exc())


    def get_queue_names(self, **kwargs):
        return list(self.queues.keys())


    def get_tasks(self, q_name, **kwargs):
        return list(self.queues[q_name].queue)


    def get_task_state(self, q_name, run_id, **kwargs):
        return self.queues[q_name].get_task_state(run_id)


    def pause(self, **kwargs):
        
        try:
            self.LOG.info('pausing')

        except: # may raise during shutdown
            pass 

        self._run.clear()

    def resume(self, **kwargs):

        if ((self.has_pending_tasks()) and \
        (not self._run.is_set())):

            self.LOG.info('resuming')

            self._run.set()


    def append(self, *args, **kwargs):

        q_name = kwargs['qname']
        payload = kwargs['info']

        print('IOScheduler append - queue: {}'.format(q_name))
        print(payload)
        print(kwargs)

        if 'run_id' in kwargs:
            run_id = kwargs['run_id']

        else:
            print('WARNING: CREATING NEW RUN ID')
            run_id = str(uuid.uuid4())
        
        # self.LOG.debug(
        #    'Adding task: {} {}'.format(q_name, payload),
        #    extra={'extra':payload}
        #)

        print(self.queues)
        print(self.queues[q_name])
        self.queues[q_name].append(run_id, payload)

        self.resume()

        return run_id


    # ------------------

    def trigger_shutdown(self):

        self.LOG.info('triggering shutdown')

        self._shutdown.set()
        self._run.set()

    def check_shutdown(self):
        return self._shutdown.is_set()

    def is_running(self):
        return self._run.is_set()

    def has_pending_tasks(self):

        if self.check_shutdown():
            return False

        for qname, q in self.queues.items():
            if len(q.queue) > 0:
                return True

        return False


    def force_quit(self):

        print('FORCE QUIT')
        self.LOG.debug('FORCE QUIT')

        self.trigger_shutdown()
        sys.exit()


    def run(self):
        
        while not self.check_shutdown():

            self._run.wait()

            try:
                self.LOG.debug('waking up')

            except: # may raise during shutdown
                pass


            while self.has_pending_tasks():
                
                for q_name, queue in self.queues.items():

                    # print('checking {}'.format(q_name))

                    if queue.has_pending_tasks():
                        with queue.dispatch_condition:
                            queue.dispatch_condition.notify()

                time.sleep(3)

            self.pause()


        print('RUN EXITING')