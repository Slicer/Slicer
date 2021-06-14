# Endoscopy

## Overview

Provides interactive animation of flythrough paths.

## Panels and their use

### Path

Inputs related to defining the path.

- **Camera**Camera: The input camera that will be controlled by the path.
- **Input Fiducials**Input Fiducials: The list of control points defining the path.
- **Create Path**Create Path: Create a path model as an interpolation of the currently selected points.  Will enable the Flythrough panel.

### Flythrough

Controls for animating along the path.

- **Frame**Frame: The current frame (step along the path).
- **Frame Skip**Frame Skip: Number of frames to skip (larger numbers make the animation go faster).
- **Frame Delay**Frame Delay: Time delay between animation frames (larger numbers make the animation go slower)
- **View Angle**View Angle: Field of view of the camera.  The default value of 30 approximates normal camera lenses.  Larger numbers, such as 110 or 120 approximate the wide angle lenses often used in endoscopy viewing systems.
- **Play/Stop**Play/Stop: Field of view of the camera.  The default value of 30 approximates normal camera lenses.  Larger numbers, such as 110 or 120 approximate the wide angle lenses often used in endoscopy viewing systems.

## Contributors

Steve Pieper (Isomics), Jean-Christophe Fillion-Robin (Kitware)

## Acknowledgements

This work is supported by PAR-07-249: R01CA131718 NA-MIC Virtual Colonoscopy (See http://www.na-mic.org/Wiki/index.php/NA-MIC_NCBC_Collaboration:NA-MIC_virtual_colonoscopy) NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See http://www.slicer.org for details. Module implemented by Steve Pieper.
