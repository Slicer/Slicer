
import string
import socket
import sys
import numpy
import pylab

vtk_types = { 2:numpy.int8, 3:numpy.uint8, 4:numpy.int16,  5:numpy.uint16,  6:numpy.int32,  7:numpy.uint32,  10:numpy.float32,  11:numpy.float64 }
numpy_sizes = { numpy.int8:1, numpy.uint8:1, numpy.int16:2,  numpy.uint16:2,  numpy.int32:4,  numpy.uint32:4,  numpy.float32:4,  numpy.float64:8 }

# create a socket
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# connect to server
host = 'localhost'
port = 18943
s.connect((host, port))

if len(sys.argv) < 2:
  print "usage: slicerget <id>"
  sys.exit(1)

id = sys.argv[1]

s.send('get ' + str(id) + ' \n') # send 

fp = s.makefile();

line = string.strip(fp.readline())
name = string.strip(fp.readline())
scalar_type = string.split(string.strip(fp.readline()))
dimensions = string.split(string.strip(fp.readline()))
space_origin = string.split(string.strip(fp.readline()))
space_directions = string.split(string.strip(fp.readline()))

print dimensions
dtype = vtk_types [ int(scalar_type[1]) ]
size = numpy_sizes [ dtype ]
print size
size = size * int(dimensions[3]) * int(dimensions[2]) * int(dimensions[1])
print size

#im = numpy.fromfile( fp, dtype, shape=(int(dimensions[3]), int(dimensions[2]), int(dimensions[1])) )
#im = numpy.fromfile( fp, dtype )


data = s.recv(size)
while len(data) != size:
  data += s.recv(size)


im = numpy.fromstring (data, dtype).reshape( int(dimensions[3]), int(dimensions[2]), int(dimensions[1]) )

print numpy.mean(im)

slice = im[16,:,:]
pylab.imshow(slice)
pylab.show()


# close the connection
s.close()

sys.exit()
