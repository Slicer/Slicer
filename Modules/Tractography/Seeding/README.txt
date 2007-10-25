Explanation of the tract seeding module...
[modeled after slicer2 LaurenThesis->SeedBrain]

This module generates tractography at every point in a DTI dataset,
according to the parameters.  

Seed Placement Options
=====================
Tracts are seeded at each point on a grid
defined by the seed spacing. Useful values are in the range of 1-3 mm.
Lower values will create many more fibers (perhaps on the order of 80,000)
in the whole brain.  About 10-20,000 fibers can usefully be viewed at any
one time for whole brain tract viewing.

The random grid option will jitter seed points within their grid cubes (so
if spacing is 2mm, a grid point can be jittered within 1mm in any direction
from its original location).  The intention here is to avoid any bias that
many be caused by the seeding on a regular grid. This is an advanced option
that is not needed by most users.

The cL threshold (under seed placement options) is a linear measure
threshold (cL == linear measure, it's C-F Westin's anisotropy measure).
This limits the seed region to points that have tensors that are above this
threshold. The reason for this is to limit seeding to the white matter. The
linear measure is: (lambda1 - lambda2)/(sum of lambdas) where lambdas are
the eigenvalues. There is also another definition with another
normalization in the denominator proposed by westin.  This measure is
specific to cigar-shaped tensors.
[cL should be changed to linear or vice versa, or both listed]

Tractography Seeding Parameters
=========================
Min Length is the minimum length in millimiters allowed for a
trajectory. Fibers shorter than this will be rejected (and not saved or
displayed).  This can be usefully set from 10mm to perhaps 80mm.

The maximum length is not particularly useful for the brain but could be
used for test data. In the brain, the cL anisotropy threshold will cut off
the trajectories rather than the length cutoff.

The Stopping mode allows the choice between anisotropy measures in order to
stop tractography. Linear (cL) and fractional anisotropy (FA) are choices.
[Planar measure should be removed]

There are two thresholds, one for starting and one for stopping
tractography. They do not need to be the same.
[ the naming of the panels could be improved to reflect starting and
stopping]

ROI definition
======
[ an ROI volume should be selectable]
This defines the region of interest within the DTI data for seeding.  It
can be a whole brain mask to avoid seeding outside the head, or a
subregion.
[ In slicer 2 I believe this seeded everywhere with a label greater than 0]
