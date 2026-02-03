
from .objects import FrameSequence

def _test():
    
    tests = [('1-10', None, None, 1),
             ('1-10', None, None, 2),
             ('1-10', 2, 5, 1),
             ('1-10', 2, 5, 2),
             ('1-10x2', None, None, 1),
             ('1-10x2,17-20', None, None, 1),
             ('1-30x', None, None, 1),
             ('1-10,15-20', None, None, 1),
             ('1-10,15-20', None, None, 10),
             ('1-10,15-20', 7, 15, 3),
             ('1-10,15-20', 7, 50, 3),
             ('15-20', 7, 50, 3),
             ('1,34,89', None, None, 1),
             ('1,34,89,210-223', None, None, 2)]
    
    
    for test_entry in tests:

        description, start, end, inc = test_entry
        
        print('---')
        print('{d} start: {s} end: {e} inc: {i}'.format(
            d=description, s=start, e=end, i=inc))
        
        try:
            s = FrameSequence(description)
            
            print('All: ' + str(s.frame_list))
            print('')
            
            for frame_num in s.iterate(start, end, inc):
                print(frame_num)
                
        except Exception as e:
            print('Got Error: {ecls} - {msg}'.format(
                ecls=e.__class__.__name__, msg=str(e)))
            


def _chunk_test():
    
    tests = [('1-10', 3),
             ('1-20x2', 3),
             ('1-5,10-16', 3),
             ('1,20,23,40', 3),
             ('1,20,23,40, 50-58', 3)]
 
    for chunk_test in tests:
        
        description, chunk_size = chunk_test
        
        print('---')
        print('{d} {s}'.format(d=description, s=chunk_size))
        
        s = FrameSequence(description)
        print(s.frame_chunks)
        
        print('Group Gaps:')
        r = s.chunk(chunk_size)
        print(r)
        
        print('Split Gaps:')
        r = s.chunk(chunk_size, group_gaps=False)
        
        print(r)
    
def _split_test():
    
    tests = [('1-10', 2),
             ('1-10', 3),
             ('1-10,15,16,21-30', 5),
             ('1-37x3', 5)]
    
    for split_test in tests:
        
        description, split_size = split_test
        
        s = FrameSequence(description)
        print(s.split(split_size))
        
        
