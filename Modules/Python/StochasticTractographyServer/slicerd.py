# module to interact with the SlicerDaemon
# (based on python_101_module_simple.py)

"""
This module is useful for accessing the SlicerDaemon.
"""
import logging
import string
import socket
import sys
import numpy
import nrrd


logger                     = logging.getLogger(__name__)

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
        logger.info( "slicerd -- host: %s  port: %d" % (self.host, self.port))

    def ls(self):
        # create a socket
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect( (self.host, self.port) )
        s.send('ls \n') # send 

        fp = s.makefile()
        
        line = string.strip(fp.readline())
        line = string.split(line)

        s.close()

        return line

    def eval(self,p):
        # create a socket
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect( (self.host, self.port) )
        s.send('eval $::slicer3::ApplicationGUI SetExternalProgress "update" ' + str(p)) # send       

        s.close()


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
        logger.debug( "Name : %s" % name)

        scalar_type = string.strip(fp.readline())
        nimage.set('scalar_type', scalar_type)
        scalar_type = string.split(scalar_type)
        logger.debug( "Type : %s" % scalar_type)

        dimensions = string.strip(fp.readline())
        nimage.set('dimensions', dimensions)
        dimensions = string.split(dimensions)

        if len(dimensions)==4:
            logger.debug( "Dimensions : %s:%s:%s" % (dimensions[1], dimensions[2], dimensions[3]))

        if len (dimensions)==5:
            logger.debug( "Dimensions : %s:%s:%s:%s" % (dimensions[1], dimensions[2], dimensions[3], dimensions[4]))

        space_origin = string.strip(fp.readline())
        nimage.set('space_origin', space_origin)
        space_origin = string.split(space_origin)

        space_directions = string.strip(fp.readline())
        nimage.set('space_directions', space_directions)
        space_directions = string.split(space_directions)

        kinds = string.strip(fp.readline())
        nimage.set('kinds', kinds)
        kinds = string.split(kinds)

        isDti = False
        if kinds[1] =='3D-masked-symmetric-matrix':
            mu = string.strip(fp.readline())
            nimage.set('measurement_frame', mu)
            mu = string.split(mu)
            isDti = True

        isDwi = False
        if kinds[1] =='vector':
            mu = string.strip(fp.readline())
            nimage.set('measurement_frame', mu)
            mu = string.split(mu)
            isDwi = True

        dtype = vtk_types [ int(scalar_type[1]) ]
        size = numpy_sizes [ dtype ]
        if isDti:
           size = size * int(dimensions[1]) * int(dimensions[4]) * int(dimensions[3]) * int(dimensions[2])
        elif isDwi:
           size = size * int(dimensions[4]) * int(dimensions[3]) * int(dimensions[2]) * int(dimensions[1])
        else:
           size = size * int(dimensions[3]) * int(dimensions[2]) * int(dimensions[1])

        data = s.recv(size)
        while len(data) != size:
          data += s.recv(size)

        im = numpy.fromstring (data, dtype)
        if isDti:
           im = im.reshape( int(dimensions[1]), int(dimensions[4]) * int(dimensions[3]) * int(dimensions[2]) )
        elif isDwi:
           im = im.reshape( int(dimensions[4]), int(dimensions[3]), int(dimensions[2]) , int(dimensions[1]) )
        else:
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
        shape = nimage.getImage().shape

        s.send('put\n') 
        s.send('image ' + name + '\n') 
       
        s.send('space right-anterior-superior\n')

        s.send(nimage.get('dimensions') + '\n') 

        s.send(nimage.get('space_origin') + '\n')

        s.send(nimage.get('space_directions') + '\n')

        s.send(nimage.get('kinds') + '\n')

        s.send('components 1\n')

        s.send('scalar_type ' + numpy_vtk_types[ str(nimage.getImage().dtype) ] + '\n')

        if nimage.hasKey('measurement_frame'):
            s.send(nimage.get('measurement_frame') + '\n')
        

        s.send(data)
        
        s.close()

    def putS(self, scal, dims, org, i2r, name='from_slicerd'):
        """put the image back into slicer
        """
        # create a socket
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect( (self.host, self.port) )

        data = scal.tostring()

        s.send('put\n') 
        s.send('image ' + name + '\n') 
        s.send('space right-anterior-superior\n')
        s.send('dimensions ' + str(dims[2]) + ' ' + str(dims[1]) + ' ' + str(dims[0]) + '\n') 
        s.send('space_origin '  + '(' + str(org[0]) + ', ' + str(org[1]) + ', ' + str(org[2]) + ')'  + '\n')
        s.send('space_directions ' + '(' + str(i2r[0,0]) + ', ' + str(i2r[1,0]) + ', ' + str(i2r[2,0]) + ')' + \
                                     '(' + str(i2r[0,1]) + ', ' + str(i2r[1,1]) + ', ' + str(i2r[2,1]) + ')' + \
                                     '(' + str(i2r[0,2]) + ', ' + str(i2r[1,2]) + ', ' + str(i2r[2,2]) + ')' + '\n')
        s.send('kinds space space space\n')
        s.send('components 1\n')
        s.send('scalar_type ' + numpy_vtk_types[ str(scal.dtype) ] + '\n')
        
        s.send(data)
        
        s.close()

    def putD(self, dti, dims, org, i2r, mu, name='from_slicerd'):
        """put the image back into slicer
        """
        # create a socket
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect( (self.host, self.port) )


        data = dti.tostring()

        s.send('put\n') 
        s.send('image ' + name + '\n') 
        s.send('space right-anterior-superior\n')
        s.send('dimensions ' + str(7) + ' ' + str(dims[2]) + ' ' + str(dims[1]) + ' ' + str(dims[0]) + '\n') 
        s.send('space_origin '  + '(' + str(org[0]) + ', ' + str(org[1]) + ', ' + str(org[2]) + ')'  + '\n')
        s.send('space_directions none ' + '(' + str(i2r[0,0]) + ', ' + str(i2r[1,0]) + ', ' + str(i2r[2,0]) + ')' + \
                                          '(' + str(i2r[0,1]) + ', ' + str(i2r[1,1]) + ', ' + str(i2r[2,1]) + ')' + \
                                          '(' + str(i2r[0,2]) + ', ' + str(i2r[1,2]) + ', ' + str(i2r[2,2]) + ')' + '\n')
        s.send('kinds 3D-masked-symmetric-matrix space space space\n')
        s.send('components 1\n')
        s.send('scalar_type ' + numpy_vtk_types[ str(dti.dtype) ] + '\n')

        s.send('measurement_frame ' + '(' + str(mu[0,0]) + ', ' + str(mu[1,0]) + ', ' + str(mu[2,0]) + ')' + \
                                      '(' + str(mu[0,1]) + ', ' + str(mu[1,1]) + ', ' + str(mu[2,1]) + ')' + \
                                      '(' + str(mu[0,2]) + ', ' + str(mu[1,2]) + ', ' + str(mu[2,2]) + ')' + '\n')

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
