# module to handle nrrd images
# (based on python_101_module_simple.py)

"""
This module is useful for manipulating nrrd images
This is a very simple starting point, but usable for slicerd
"""

import string
import sys
import numpy

LABEL = '===== NRRD Module ====='

class nrrd:
    """simple class definition.
    """
    def __init__(self):
        self.keys = {}
        self.im = numpy.empty(0)

    def show(self):
        print 'nrrd'

    def set(self, key, value):
        """
        """
        self.keys[key] = value

    def get(self, key):
        """
        """
        return self.keys[key]

    def setImage(self, im):
        """
        """
        self.im = im

    def getImage(self):
        return self.im

def main():
    """
    A test harness for this module.
    """
    print LABEL
    n = nrrd()
    n.set('origin', '(0, 0, 0)')
    n.setImage(numpy.array([1, 2, 3]))
    print 'origin is ' + n.get('origin')
    print 'image is ' + str(n.getImage())

if __name__ == '__main__':
    main()
