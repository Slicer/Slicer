import numpy as np
import scipy.signal as sciS

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
    sigma = FWHM[k]/dim[k]/2.35          # sigma in voxel units
    l = np.floor(3*sigma+1)
    f = np.exp(-np.arange(-l,l+1)[:, np.newaxis]**2/(2*sigma**2))
    f = f/np.sum(f)

    s = x.shape
  
    ## Reshape data (Matlab specific)
    x = np.reshape(x,(s[0], np.prod(s)/s[0]))
  
    # Perform 1D convolution
    x = sciS.convolve2d(f,x,'same')
  
    # Reshape and shift dims
    x = np.reshape(x,s)
    x = x.swapaxes(1,0)
    x = x.swapaxes(2,1)
    
  return x
