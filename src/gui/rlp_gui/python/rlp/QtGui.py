
import traceback


LEFT_BUTTON = 1
RIGHT_BUTTON = 2


try:
    from RlpGuiQTGUI import *

except:
    print(traceback.format_exc())
