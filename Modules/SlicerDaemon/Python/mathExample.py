
import slicerd
import numpy

s = slicerd.slicerd()

n = s.get(0)

im = n.getImage()
n.setImage( im * im )

s.put(n, 'newImage')

