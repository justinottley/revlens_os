
import os
import logging
import traceback

class NullHandler(logging.Handler):

    def emit(self, logrecord):
        pass

class RlpLogger(logging.Logger):
    
    def __init__(self, *args, **kwargs):
        logging.Logger.__init__(self, *args, **kwargs)
        
        # register a null handler so there is at least one handler registered -
        # prevents errors that expect handlers
        
        self.addHandler(NullHandler())
        
        
    def notice(self, message, *args, **kwargs):
        return self.log(logging.NOTICE, *args, **kwargs)
        
    def attention(self, message, *args, **kwargs):
        return self.log(logging.ATTENTION, *args, **kwargs)
        
    def verbose(self, message, *args, **kwargs):
        return self.log(logging.VERBOSE, message, *args, **kwargs)
        
    def status(self, message, *args, **kwargs):
        return self.log(logging.STATUS, message, *args, **kwargs)
        
    def progress(self, value, *args, **kwargs):
        return self.log(logging.PROGRESS, value, *args, **kwargs)




def _init_base():

    # register additional loglevels
    logging.addLevelName(5, 'VERBOSE')
    logging.addLevelName(9, 'VERBOSE')
    logging.VERBOSE = 5
    
    logging.addLevelName(11, 'PROGRESS')
    logging.PROGRESS = 11
    
    logging.addLevelName(21, 'STATUS')
    logging.STATUS = 21
    
    logging.addLevelName(31, 'NOTICE')
    logging.NOTICE = 31
    
    logging.addLevelName(32, 'ATTENTION')
    logging.ATTENTION = 32
    
    # Register our logger with logging
    logging.setLoggerClass(RlpLogger)
    
    
    rlogger = logging.getLogger('rlp')
    
    # prevent propagation to the root logger. This prevents the rlp logger
    # from spewing a lot of logs where we don't want to see. For example
    # the maya script editor
    rlogger.propagate = False


def _init_rlp_logging():

    import RlpCoreUTILmodule

    class RlpHandler(logging.Handler):

        def emit(self, logrecord):
            msg = '[Py] {} [ {} ] {}'.format(
                str(logrecord.name).ljust(25, ' '),
                str(logrecord.levelname).ljust(8, ' '),
                logrecord.msg
            )
            # RlpCoreUTILmodule.UTIL_LogUtil.handleLog(msg)
            print(msg)

    def _rlp_log_print(msg_in, *args, **kwargs):

        stack = traceback.extract_stack()[:-1]
        last = stack[-1]

        module = last.filename
        if 'site-packages' in module:
            module = module.split('site-packages')[1].replace('/', '.').replace('\\', '.').replace('.py', '')[1:]
        else:
            module = os.path.split(module)[-1]

        msg = '[Py] {}:{} - {}'.format(module, last.lineno, msg_in)
        # RlpCoreUTILmodule.UTIL_LogUtil.handleLog(msg)
        print(msg)


    # __builtins__['_print'] = __builtins__['print']
    # __builtins__['print'] = _rlp_log_print

    rlp_handler = RlpHandler()
    rlp_handler.setLevel(1) # logging.VERBOSE)
    for logname in ['rlp', 'scaffold', 'sclib']:
        l = logging.getLogger(logname)
        l.setLevel(1)
        l.addHandler(rlp_handler)




def _init_py():

    import tempfile
    import logging.handlers

    _init_base()

    formatter = logging.Formatter(
        '%(asctime)-26s %(name)-30s [ %(levelname)-8s ] %(message)s')
    
    logfile_userdir = os.path.join(
        tempfile.gettempdir(),
        os.getenv('USER', os.getenv('USERNAME'))) # Windows and UNIX
        
    if not os.path.isdir(logfile_userdir):
        os.mkdir(logfile_userdir)
        
    logfile = os.path.join(logfile_userdir, 'rlp.log')
    if os.name == 'nt':
        logfile += '.' + str(os.getpid())
    
    if os.getenv('RLP_LOGGING_SUFFIX'):
        logfile += os.getenv('RLP_LOGGING_SUFFIX')
     
    file_handler = logging.handlers.RotatingFileHandler(
        logfile, maxBytes=10048576, backupCount=5)
    
    file_handler.setFormatter(formatter)
    file_handler.setLevel(logging.VERBOSE)
    
    rlogger = logging.getLogger('rlp')
    rlogger.addHandler(file_handler)
    rlogger.setLevel(logging.VERBOSE)

    rlogger.info('Logging initialized')


def _init():
    _init_base()


def basic_config(logname='rlp', loglevel=None):

    stdout_handler = handlers.get_streamhandler()
    if loglevel:
        stdout_handler.setLevel(loglevel)
        
    rlogger = logging.getLogger(logname)
    rlogger.addHandler(stdout_handler)


_init()
del _init
