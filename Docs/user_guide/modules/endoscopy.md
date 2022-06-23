# Endoscopy

## Overview

Provides interactive animation of flythrough paths.

## Basic use
Use the Markups module or toolbar to create a set of points in 3D space.  These then need to be selected to be the Input Fiducials (Point List).  Then create a new target model for the path.  Clicking Create Path will populate the selected model with a curve through the input points.  There will also be a Cursor node showing the current camera position.  You may need to turn off the visibility of the points and cursor (using the Data module) if they obscure the view in 3D.  The Flythrough controls can be used to view the simulated endoscopy. 

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

This work is supported by PAR-07-249: R01CA131718 NA-MIC Virtual Colonoscopy (See https://www.na-mic.org/Wiki/index.php/NA-MIC_NCBC_Collaboration:NA-MIC_virtual_colonoscopy) NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See https://www.slicer.org for details. Module implemented by Steve Pieper.
