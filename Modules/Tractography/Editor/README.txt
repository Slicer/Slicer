[ in slicer2 this is LaurenThesis->ROISelect. Katerina is maintaining it
for the pnl]

This module selects fiber trajectories that pass through ROIs. They are
output as a new fiber bundle.  This is done by using a vtkProbeFilter to
sample the ROI volume at the points along the trajectories.  Then
those trajectories which have nonzero values are selected.

[ currently the module appends its input together and does not do the
selection. it does the probing. also this will break if things are within
transforms, as the nrrd headers only know about RAS->ijk currently. the
polydata is assumed to be in Ras/world and the nrrd header of the ROI knows
ijk->RAS. The two RAS systems are assumed for now to align.]

[also this uses vtk CLI due to it operating on polydata]

IO
====
input ROIs  should be nonzero where one would like to select
[we'd like three I think like in slicer2?]
[we'd like negative (NOT), these are AND.. more boolean ops]

ROI definition
====
I have no idea why this is here.

