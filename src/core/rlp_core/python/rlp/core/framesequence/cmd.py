
import logging
from .objects import FrameSequence

rlogger = logging.getLogger('rlp.{}'.format(__name__))

def get_frames(description, task_num, task_count):
    
    all_frames = FrameSequence(description)
    
    frame_chunks = all_frames.split(task_count)
    frame_chunk = frame_chunks[task_num - 1]
    
    return frame_chunk
    
    
def get_frame_description(frame_list):
    '''
    Generate a string frame description based on the input list of frame
    numbers
    '''
    
    franges = []
    
    xnum = 0
    
    start_iter_num = None
    end_iter_num = None
    
    
    for entry in frame_list:
        
        try:
            int_entry = int(entry)
            
            if start_iter_num is None:
                start_iter_num = int_entry
                xnum = start_iter_num
                
            elif (int_entry != xnum):
                
                # end of this continuous sequence
                # end_iter_num = int_entry
                
                if start_iter_num != end_iter_num:
                    franges.append((start_iter_num, end_iter_num))
                    
                else:
                    franges.append((start_iter_num,))
                
                # reset a new start position
                start_iter_num = int_entry
                xnum = start_iter_num
            
            end_iter_num = int_entry
            
            
        except:
            
            # end of this continouous sequence because of letter insert
            if start_iter_num is not None:
                if start_iter_num != end_iter_num:
                    franges.append((start_iter_num, end_iter_num))
                    
                else:
                    franges.append((start_iter_num,))
            
            
            start_iter_num = None
            
            franges.append((entry,))
            
            
        xnum += 1
    
    if start_iter_num is not None:
        franges.append((start_iter_num, end_iter_num))
    
    # format the result
    str_entries = []
    for frange_entry in franges:
        
        if len(frange_entry) == 2:
            
            if frange_entry[0] == frange_entry[1]:
                str_entries.append(str(frange_entry[0]))
                
            else:
                str_entries.append('{start}-{end}'.format(start=frange_entry[0],
                                                          end=frange_entry[1]))
                
        else:
            
            assert(len(frange_entry) == 1)
            str_entries.append(str(frange_entry[0]))
            
    
    str_result = ','.join(str_entries)
    
    return str_result


def get_fml_description(sequence):
    '''
    Return a frame description string of the first,middle,last frames
    of input sequence. Input can be a string frame description or a sequence
    object
    '''
    
    if issubclass(sequence.__class__, str):
        sequence = FrameSequence(sequence)
        
    f_frame = sequence.frame_list[0]
    my_frame = sequence.frame_list[int(len(sequence.frame_list) / 2)]
    life_frame = sequence.frame_list[-1]
    
    fml_description = '{f},{my},{life}'.format(
        f=f_frame,
        my=my_frame,
        life=life_frame)
    
    return fml_description
    
    
def get_fml_sequence(sequence):
    '''
    Return a sequence object of the first,middle,last frames of input sequence
    '''
    
    return FrameSequence(get_fml_description(sequence))
    
    
