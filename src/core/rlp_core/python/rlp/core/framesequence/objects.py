
import logging

from rlp.core import rlp_logging

rlogger = logging.getLogger('rlp.{}'.format(__name__))

class FrameSequenceError(Exception): pass

class FrameSequence(object):
    
    SORT_INCREASING = 0
    SORT_DECREASING = 1
    SORT_NONE = 2
    
    def __init__(self, description, sort=SORT_INCREASING):
        
        self._description = description
        self.frame_list = []
        self.frame_chunks = []
        self.sort = sort
        
        self._parse()
        
        
    def __len__(self):
        return len(self.frame_list)
        
    def __str__(self):
        return self._description
        
    def __iter__(self):
        return self.iterate()
        
        
    def _parse(self):
        
        frame_chunks = self._description.split(',')
        for frame_chunk in frame_chunks:
            
            frame_chunk_list = []
            
            inc = 1
            if 'x' in frame_chunk:
                frame_chunk_parts = frame_chunk.split('x')
                if len(frame_chunk_parts) != 2 or not frame_chunk_parts[1]:
                    raise FrameSequenceError('Invalid format for fixed increment in frame range chunk: {ch}'.format(
                        ch=frame_chunk))
                
                frame_chunk, inc = frame_chunk_parts
                inc = int(inc)
                
                
            if '-' in frame_chunk: # frame range
                
                chunk_parts = frame_chunk.split('-')
                if len(chunk_parts) != 2:
                    raise FrameSequenceError('Invalid format for frame range chunk: {ch}'.format(
                        ch=frame_chunk))
                    
                chunk_start, chunk_end = [int(float(cp)) for cp in chunk_parts]
                
                # not good for memory on huge ranges..
                all_frames = list(range(chunk_start, (chunk_end + 1))) 
                num_all_frames = len(all_frames)
                
                frame_idx = 0
                while frame_idx < len(all_frames):
                    
                    self.frame_list.append(all_frames[frame_idx])
                    frame_chunk_list.append(all_frames[frame_idx])
                    
                    frame_idx += inc
                    
                    
                    
            else:
                
                self.frame_list.append(int(frame_chunk))
                frame_chunk_list.append(int(frame_chunk))
                
                
            self.frame_chunks.append(frame_chunk_list)
            
            
        if self.sort == self.SORT_INCREASING:
            self.frame_list = sorted(self.frame_list)
            
        elif self.sort == self.SORT_DECREASING:
            self.frame_list = reversed(sorted(self.frame_list))
            
    
    @property
    def ordered(self):
        return self.sort in [self.SORT_INCREASING, self.SORT_DECREASING]
        
        
    def _chunk_group_gaps(self, chunk_size, inc=1):
        
        result = []
        
        frame_idx = 0
        
        frame_list_size = len(self.frame_list)
        while (frame_idx < frame_list_size):
            
            chunk_frames = []
            for chunk_idx in range(chunk_size):
                
                if frame_idx >= frame_list_size:
                    break
                    
                chunk_frames.append(self.frame_list[frame_idx])
                frame_idx += inc
                
            result.append(chunk_frames)
            
        return result
        
        
    def _chunk_split_gaps(self, chunk_size, inc=1):
        
        result = []
        
        for frame_group in self.frame_chunks:
            
            frame_group_idx = 0
            frame_group_size = len(frame_group)
            
            while frame_group_idx < frame_group_size:
                
                chunk_group = []
                
                for chunk_idx in range(chunk_size):
                    
                    if frame_group_idx >= frame_group_size:
                        break
                        
                    
                    chunk_group.append(frame_group[frame_group_idx])
                    frame_group_idx += inc
                    
                
                result.append(chunk_group)
            
        return result
        
        
    def chunk(self, chunk_size, inc=1, group_gaps=True):
        
        if group_gaps:
            return self._chunk_group_gaps(chunk_size, inc)
            
        else:
            return self._chunk_split_gaps(chunk_size, inc)
        
        
        
    def split(self, num_groups):
        '''
        Split the sequence into input number of groups
        '''
        
        all_groups = []
        
        frame_list_size = len(self.frame_list)
        chunk_size = frame_list_size / float(num_groups)
        int_chunk_size = int(round(chunk_size))
        
        rlogger.debug('splitting - frame count: {f} num groups: {n} chunk size: {s}'.format(
            f=frame_list_size,
            n=num_groups,
            s=chunk_size))
        
        
        frame_idx = 0
        group_count = 0
        
        while frame_idx < frame_list_size:
            
            group = []
            
            for chunk_idx in range(int_chunk_size):
            
                if frame_idx >= frame_list_size:
                    break
                    
                group.append(self.frame_list[frame_idx])
                
                frame_idx += 1
                
            group_count += 1
            
            if group_count <= num_groups:
                all_groups.append(group)
                
            else:
                all_groups[-1].extend(group)
                
        
        
        return all_groups
        
        
    def iterate(self, start=None, end=None, inc=1):
        
        if ((start is not None) or (end is not None)) and not self.ordered:
            raise FrameSequenceError('iterate start/end not allowed on unordered sequence')
            
        
        num_frames = len(self.frame_list)
        
        start_idx = 0
        if start: # skip to start or next number after start
            while start_idx < num_frames:
                
                if start > self.frame_list[start_idx]:
                    start_idx += 1
                    
                else:
                    break
                  
            else:
                # is it possible to reach here?
                raise FrameSequenceError('Invalid start')
        
        
        end_idx = start_idx
        if end:
            while end_idx < num_frames:
                
                if end > self.frame_list[end_idx]:
                    end_idx += 1
                    
                else:
                    break
            else:
                
                rlogger.warning('Clamping end to last value')
                end_idx = num_frames - 1
            
        else:
            end_idx = num_frames - 1
            
            
        frame_index = start_idx
        
        while frame_index <= end_idx:
            yield self.frame_list[frame_index]
            
            frame_index += inc
            
            
    


