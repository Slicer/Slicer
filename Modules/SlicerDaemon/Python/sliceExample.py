
import slicerd
import pylab

s = slicerd.slicerd()

n = s.get(0)

slice = n.getImage()[16,:,:]
pylab.imshow(slice)
pylab.show()
