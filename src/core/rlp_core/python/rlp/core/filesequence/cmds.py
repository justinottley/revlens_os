
import os
import glob
import logging

rlogger = logging.getLogger('rlp.{}'.format(__name__))

from rlp.core import filesequence

def reversion(input_sequence, new_version):
    '''
    Replace the version number of the input sequence with the new version.
    
    There is a workflow specific naming convention at work here, which is
    that the version number is always before a dot
    '''
    
    rlogger.info('reversioning sequence')
    
    result = []
    for entry in input_sequence:
        
        new_item = entry.template.format(f=entry.f, version=new_version)
        result.append(os.path.join(entry.dirname, new_item))
        
    return filesequence.Sequence(result)
    
    
def offset(input_sequence, offset):
    '''
    Offset an input sequence by a number
    '''
    
    rlogger.info('offsetting sequence')
    
    result = []
    for entry in input_sequence:
        
        new_f = str(int(entry.f) + offset).zfill(entry.padding)
        new_item = entry.template.format(f=new_f, version=entry.version)
        result.append(os.path.join(entry.dirname, new_item))
            
    return filesequence.Sequence(result)
    
    
def resequence(input_sequence, start_num=0, padding=4):
    '''
    resequence an input sequence with sequential numbers. This is useful
    mostly if the sequence has letter inserts, and a new sequence is
    desired without inserts
    '''
    
    rlogger.info('resequencing sequence')
    
    result = []
    
    count = 1
    for entry in input_sequence:
        
        new_f = str(count + start_num).zfill(padding)
        new_item = entry.template.format(f=new_f, version=entry.version)
        result.append(os.path.join(entry.dirname, new_item))
        
        count += 1
        
    return filesequence.Sequence(result)
    
    
def get_sequences(source, ignore_items=['Thumbs.db'], ignore_dirs=True):
    
    rlogger.verbose('get_sequences(): {s}'.format(s=source))
    
    items = []
    
    sequences = []
    
    source_type = type(source)
    
    def _remove_ignore_items(item_list):
        
        for ignore_item in ignore_items:
            ignore_path = os.path.join(source, ignore_item)
            if ignore_path in item_list:
                
                rlogger.verbose('Removing {i} from sequence at {p}'.format(
                    i=ignore_item,
                    p=source))
                
                item_list.remove(ignore_path)
    
    
    
    if source_type == list:
        items = source
        
    elif source_type in [str, str] and os.path.isdir(source):
        items = os.listdir(source)
        _remove_ignore_items(items)
        
    elif source_type in [str, str]:
        items = glob.glob(source)
        _remove_ignore_items(items)
        
    else:
        raise TypeError('Unsupported format for source argument: {type_str}'.format(
            type_str=source_type))



    for item in items:
        
        if ignore_dirs and os.path.isdir(item):
            continue
            
            
        found = False
        for seq in sequences:
            
            if type(item) is not filesequence.Item:
                item = filesequence.Item(item)
                
            (result, _) = seq.match(item)
            if result:
                seq.append(item)
                found = True
                
        if not found:
            seq = filesequence.Sequence()
            seq.append(item)
            
            sequences.append(seq)
            
    return sequences
    
    
def get_sequence_from_template_string(file_str):
    
    file_dir, file_template = os.path.split(file_str)
        
    rlogger.info('analyzing file sequences at {file_dir} template {ft} ..'.format(
        file_dir=file_dir,
        ft=file_template))
    
    sequences = get_sequences(file_dir)
    
    rlogger.verbose('Parsed sequence result: {seq}'.format(seq=sequences))
    
    if len(sequences) == 0:
        raise filesequence.SequenceError('no sequences found at {file_dir}'.format(
            file_dir=file_dir))
    
    if '{f}' not in file_template and '{version}' not in file_template:
        
        msg = 'invalid input file sequence for element: {file_template} '
        msg += 'expect "{{f}}" and/or "{{version}}"'
        msg = msg.format(file_template=file_template)
        raise filesequence.SequenceError(msg)
    
    seq = None
    for sequence in sequences:
        
        rlogger.verbose('Comparing {l} to {r}'.format(l=sequence.template, r=file_template))
        
        if sequence.template == file_template:
            seq = sequence
            
    if seq is None:
        raise filesequence.SequenceError('file sequence not found: {file_str}'.format(
            file_str=file_str))
    
    return seq
    
    


def register_hint(hint_name, hint_regex, offset=0):
    '''
    Registers a sequencing hint for identifying a filesequence using a single
    file. Specify a hint name and an input regular expression for matching.
    The regular expression must have a "frame" group name for identifying
    the iteration frame group.
    
    E.g., to register a hint to find files of the form prefix_{f}_suffix.ext:
    >>> register_hint("my_hint", "(.+)(?P<frame>\d+)_(\w+)")
    '''
    
    user_hint = filesequence.UserSignatureHint(hint_name, hint_regex, offset)
    filesequence.Sequence._HINTS.append(user_hint)
    
    
def remove_hint(hint_name):
    '''
    remove (unregister) a sequencing hint
    '''
    
    hint_idx = 0
    hint_obj = None
    
    for hint in filesequence.Sequence._HINTS:
        if hint.name == hint_name:
            hint_obj = hint
            break
    else:
        rlogger.error('hint "{name}" not found'.format(name=hint_name))
        
        
    if hint_obj:
        filesequence.Sequence._HINTS.remove(hint_obj)
        
