# Endoscopy

## Overview

Provides interactive animation of flythrough paths.

## Basic use
Select the `Dual 3D` view in 3D Slicer so that you will be able to witness the flythrough from both first-person and third-person viewpoints, as well as in the displayed 2d-slice panes.  Use the `Markups` module or toolbar to create a `curve` or `closed curve` in 3D space.  You are now ready to use the `Endoscopy` module.

## Panels and their use

### Path

Selects the camera and curve to be manipulated.

- **Camera**: The camera used for the flythrough.
- **Curve to modify**: The curve defining the flythrough path control points.

### Flythrough

Controls the animation.

- **Play flythrough** / **Stop flythrough**: Start or stop the flythrough animation.
- **Frame**: The current frame (step along the path).
- **Frame Skip**: Number of frames to skip (larger numbers make the animation go faster).
- **Frame Delay**: Time delay between animation frames (larger numbers make the animation go slower)
- **View Angle**: Field of view of the camera.  The default value of 30 degrees approximates normal camera lenses.  Larger numbers, such as 110 or 120 degrees approximate the wide angle lenses often used in endoscopy viewing systems.
- **Save keyframe orientation** / **Update keyframe orientation**: Press to indicate that a flythrough frame is a keyframe and that you have selected your desired camera orientation for this keyframe in the first-person, 3D viewing pane.  If you wish to update the orientation for a keyframe, use the Frame, First, Back, Next, or Last buttons to go to the frame, adjust the camera orientation, and hit this button again.
- **Delete keyframe orientation**: Discard the camera orientation associated with the selected keyframe.
- **First**: Press to go to the lowest-numbered keyframe.
- **Back**: Press to move backwards through the flythrough to the nearest keyframe.
- **Next**: Press to move forwards through the flythrough to the nearest keyframe.
- **Last**: Press to go to the highest-numbered keyframe

By default, both the flythrough path and the cursor that indicates the current position are not shown in the first-person 3d-viewing pane, but are shown in the other panes.  You can turn the visibility of the path or cursor on or off in any of these panes using the `Data` module.

### Advanced

This functionality is retained to support older workflow requiring a model.  It is expected that current users will not use the features in this section.  Instead, a curve that is modified as above and then saved will retain its keyframe information and can be loaded and used at a later time.

- **Output model**: Select a name for the model to be exported
- **Export as model**: The flythrough will be exported as a model.

## Contributors

Authors:
- Steve Pieper (Isomics)
- Jean-Christophe Fillion-Robin (Kitware)
- Harald Scheirich (Kitware)
- Lee Newberg (Kitware).

## Acknowledgements

This work is supported by PAR-07-249: R01CA131718 NA-MIC Virtual Colonoscopy (See https://www.na-mic.org/Wiki/index.php/NA-MIC_NCBC_Collaboration:NA-MIC_virtual_colonoscopy) NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See https://www.slicer.org for details.
