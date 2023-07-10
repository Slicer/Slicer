# Reformat

## Overview

This module is used for changing the slice properties.

## Panels and their use

- **Slice**: Select the slice to operate on -- the module's Display interface will change to show slice parameters.

- **Display - Edit**: Information about the selected slice. Fields can be edited to precisely set values to the slice.
  - **Offset**: See and Set the current distance from the origin to the slice plane
  - **Origin**: The location of the center of the slice. It is also related to the reformat widget origin associated to the selected slice.
    - **Center**: This button will adjust the slice Origin so that the entire slice is centered around 0,0,0 in the volume space.
  - **Orientation**
    - **Reset to**: Reset the slice to transformation to the corresponding orientation preset, such as "Axial, "Sagittal", "Coronal" or "Reformat".
    - **Rotate to volume plane**: Rotates the slice view to be aligned with the axes of the displayed volume.
    - **Flip H** and **Flip V**: Flip the image slice horizontally or vertically.
    - **Rotate CW** and **Rotate CCW**: Rotate the slice in-plane by 90 degrees in clockwise or counterclockwise direction.
    - **Normal**: Allow to set the slice plane normal direction.
      - **Normal to LR**: Set the normal to left-right anatomical direction.
      - **Normal to PA**: Set the normal to posterior-anterior anatomical direction.
      - **Normal to IS**: Set the normal to inferior-superior anatomical direction.
    - **Normal to camera**: Align the slice normal to the camera view direction.
    - **Rotation**
      - **Horizontal**: Free rotation of the slice around its horizontal axis.
      - **Vertical**: Free rotation of the slice around its vertical axis.
      - **In-Plane**: Free in-plane rotation of the slice (around its normal).

## Contributors

Michael Jeulin-Lagarrigue (Kitware)

## Acknowledgements

This work is part of the National Alliance for Medical Image Computing (NAMIC), funded by the National Institutes of Health through the NIH Roadmap for Medical Research, Grant U54 EB005149.
