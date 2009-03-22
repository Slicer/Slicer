import logging
import time
from numpy import floor, exp, arange, reshape, convolve, sum  

logger                   = logging.getLogger(__name__)

def smooth(x,FWHM,dim):
#
#     y = smooth(x,FWHM,dim)
#
#     Applies a separable Gaussian smoothing to the
#     data volume in x.
#
#     x    - input volume
#     FWHM - Full Width at Half Maximum ~ 2.35*sigma
#     dim  - Voxel dimensions, eg in millimeters


# To apply 1D convolutions, reshape the data
# to a 2D matrix so that the convolutions can be performed
# in the first dimension (Matlab convention)
  for k in range(x.ndim):
    # Create 1D filter kernel
    sigma = FWHM[k]/abs(dim[k])/2.35          # sigma in voxel units
    l = floor(3*sigma+1)
    f = exp(-arange(-l,l+1)**2/(2*sigma**2))
    f = f/sum(f)
    
    s = x.shape
    x = reshape(x,(s[0], s[1]*s[2]))

    # Perform 1D convolution
    x = convolve(f, x.flatten(), 'same')

    # Reshape and shift dims
    x = reshape(x,s)
    
  return x
