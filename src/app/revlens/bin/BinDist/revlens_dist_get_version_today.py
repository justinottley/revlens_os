import os
import sys
import datetime

def main():

    now = datetime.datetime.today()
    dist_ver = '{y}.{m}.{d}'.format(y=str(now.year)[2:], m=str(now.month).zfill(2), d=str(now.day).zfill(2))

    print(dist_ver)

if __name__ == '__main__':
    main()
