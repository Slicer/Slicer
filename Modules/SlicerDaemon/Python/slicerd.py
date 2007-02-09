# module to interact with the SlicerDaemon
# (based on python_101_module_simple.py)

"""
This module is useful for accessing the SlicerDaemon.
"""

import string
import socket
import sys
import numpy
import nrrd

LABEL = '===== Slicerd Module ====='

vtk_types = { 2:numpy.int8, 3:numpy.uint8, 4:numpy.int16,  5:numpy.uint16,  6:numpy.int32,  7:numpy.uint32,  10:numpy.float32,  11:numpy.float64 }

numpy_sizes = { numpy.int8:1, numpy.uint8:1, numpy.int16:2,  numpy.uint16:2,  numpy.int32:4,  numpy.uint32:4,  numpy.float32:4,  numpy.float64:8 }

numpy_nrrd_names = { 'int8':'char', 'uint8':'unsigned char', 'int16':'short',  'uint16':'ushort',  'int32':'int',  'uint32':'uint',  'float32':'float',  'float64':'double' }

numpy_vtk_types = { 'int8':'2', 'uint8':'3', 'int16':'4',  'uint16':'5',  'int32':'6',  'uint32':'7',  'float32':'10',  'float64':'11' }

class slicerd:
    """simple class definition.
    """
    def __init__(self, host='localhost', port=18943):
        self.host = host
        self.port = port
    def show(self):
        print 'slicerd -- host: %s  port: %d' % (self.host, self.port)

    def get(self, id):
        """get and return the image from slicer with the given id
        """
        # create a socket
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect( (self.host, self.port) )
        s.send('get ' + str(id) + ' \n') # send 

        nimage = nrrd.nrrd()

        fp = s.makefile();
        line = string.strip(fp.readline())

        name = string.strip(fp.readline())
        nimage.set('name', name)
        name = string.split(name)

        scalar_type = string.strip(fp.readline())
        nimage.set('scalar_type', scalar_type)
        scalar_type = string.split(scalar_type)

        dimensions = string.strip(fp.readline())
        nimage.set('dimensions', dimensions)
        dimensions = string.split(dimensions)

        space_origin = string.strip(fp.readline())
        nimage.set('space_origin', space_origin)
        space_origin = string.split(space_origin)

        space_directions = string.strip(fp.readline())
        nimage.set('space_directions', space_directions)
        space_directions = string.split(space_directions)

        dtype = vtk_types [ int(scalar_type[1]) ]
        size = numpy_sizes [ dtype ]
        size = size * int(dimensions[3]) * int(dimensions[2]) * int(dimensions[1])

        data = s.recv(size)
        while len(data) != size:
          data += s.recv(size)

        im = numpy.fromstring (data, dtype)
        im = im.reshape( int(dimensions[3]), int(dimensions[2]), int(dimensions[1]) )

        s.close()

        nimage.setImage(im)
        return nimage

    def put(self, nimage, name='from_slicerd'):
        """put the image back into slicer
        """
        # create a socket
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect( (self.host, self.port) )

        data = nimage.getImage().tostring()

        s.send('put\n') 
        s.send('image ' + name + '\n') 
        shape = nimage.getImage().shape
        s.send('dimensions ' + str(shape[2]) + ' ' + str(shape[1]) + ' ' + str(shape[0]) + '\n') 
        s.send(nimage.get('space_origin') + '\n')
        s.send(nimage.get('space_directions') + '\n')
        s.send('components 1\n')
        s.send('scalar_type ' + numpy_vtk_types[ str(nimage.getImage().dtype) ] + '\n')
        s.send(data)
        
        s.close()

def main():
    """
    A test harness for this module.
    """
    print LABEL
    s = slicerd()
    n = s.get(0)
    im = n.getImage()
    print 'shape of image 0 is ' + str(im.shape)
    print 'mean of image 0 is ' + str(numpy.mean(im))
    print 'origin of image is ' + n.get('space_origin')
    n.setImage(im*im)
    s.put(n, 'squared_image')

if __name__ == '__main__':
    main()
