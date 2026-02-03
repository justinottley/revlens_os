
import json
import logging


def get_formatter():
    
    return logging.Formatter(
        '%(asctime)s %(name)-28s [ %(levelname)-8s ] %(message)s')
    
    
class JSONFormatter(logging.Formatter):
    
    def _format_exc_info(self, value):
        
        result = None
        
        if value and len(value) == 3:
            exc_cls, exc_obj, traceback = value
            
            # NOTE: if exc_info is present in a logrecord, so will exc_text, so
            # we don't have to serialize the traceback again
            
            result = [exc_cls.__name__, str(exc_obj)]
            
        return result
        
        
    def format(self, logrecord):
        
        filtered_dict = {}
        ignore_attribs = ['msg', 'args']
        for key, value in list(logrecord.__dict__.items()):
            if not callable(getattr(logrecord, key)):
                
                key_formatter_func = '_format_{k}'.format(k=key)
                
                # dispatch to a specific key handler to serialize if found
                if hasattr(self, key_formatter_func):
                    filtered_dict[key] = getattr(self, key_formatter_func)(value)
                    
                else:
                    filtered_dict[key] = value
            
        return json.dumps(filtered_dict)
        
