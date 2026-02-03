

class AccumulationListener(object):
    
    def __init__(self):
        self.lines = []
        
    def __call__(self, line):
        
        line = line.rstrip()
        if line:
            self.lines.append(line)
