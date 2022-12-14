# Reformat

## Overview

This module is used for changing the slice properties.

## Panels and their use

- **Slice**: Select the slice to operate on -- the module's Display interface will change to show slice parameters.

- **Display - Edit**: Information about the selected slice. Fields can be edited to precisely set values to the slice.
  - **Offset**: See and Set the current distance from the origin to the slice plane
  - **Origin**: The location of the center of the slice. It is also related to the reformat widget origin associated to the selected slice.
  - **Center**: This button will adjust the slice Origin so that the entire slice is centered around 0,0,0 in the volume space.
  - **Normal**: Allow to set accurately the normal of the active slice.
  - **Reset**: Reset the slice to transformation to the corresponding orientation -- The orientation could be either "Axial, "Sagittal", "Coronal" or "Reformat".
  - **Normal X**: Set the normal to a x axis.
  - **Normal Y**: Set the normal to a y axis.
  - **Normal Z**: Set the normal to a z axis.
  - **Normal to camera**: Set slice normal to the camera.
  - **LR**: Rotate the slice on the X axis
  - **PA**: Rotate the slice on the Y axis
  - **IS**: Rotate the slice on the Z axis

## Contributors

Michael Jeulin-Lagarrigue (Kitware)

## Acknowledgements

This work is part of the National Alliance for Medical Image Computing (NAMIC), funded by the National Institutes of Health through the NIH Roadmap for Medical Research, Grant U54 EB005149.
