# Volume rendering

## Overview

Volume rendering (also known as volume ray casting) is a visualization technique for displaying image volumes as 3D objects directly - without requiring segmentation.

This is accomplished by specifying color and opacity for each voxel, based on its image intensity. Several presets are available for this mapping, for displaying bones,soft tissues, air, fat, etc. on CT and MR images. Users can fine-tune these presets for each image.

## Use cases

### Display a CT or MRI volume

- Load a data set (for example, use `Sample Data` module to load `CTChest` data set)
- Go to `Data` module and right-click on the eye icon, and choose "Show in 3D views as volume rendering"
- Right-click on the eye icon and choose "Volume rendering options" to switch to edit visualization options in Volume rendering module
- Choose a different preset in Display section,
- Adjust "Offset" slider to change what image intensity range is visible

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_volumerendering_basic.png)

### Render different volumes in two views

- Open the "Inputs" section
- Select the first volume
- Click View list and uncheck "View2" (only "View1" should be checked)
- Click the eye icon for the volume to show up in View1
- Select the second volume
- Click View list and uncheck "View1" (only "View2" should be checked)
- Click the eye icon for the volume to show up in View2

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_volumerendering_multiview.png)

## Limitations

- Only single-component scalar volumes can be used for volume rendering. [Vector to Scalar Volume](Module_VectorToScalarVolume) module can convert vector volume to scalar volume.
- To render multiple overlapping volumes, select "VTK Multi-Volume" rendering in "Display" section. Currently, no cropping can be applied in this mode.
- To reduce staircase artifacts during rendering, choose enable "Surface smoothing" in Advanced/Techniques/Advanced rendering properties section, or choose "Normal" or "Maximum" as quality.

## Panels and their use

- **Inputs:** Contains the list of nodes required for VolumeRendering. It is unlikely that you need to interact with controllers.
  - Volume: Select the current volume to render. Note that only one volume can be rendered at a time.
  - Display: Select the current volume rendering display properties. Volume rendering display nodes contains all the information relative to volume rendering. They contain pointers to the ROI, volume property and view nodes. A new display node is automatically created if none exist for the current volume.
  - ROI: Select the current ROI to optionally crop with 6 planes the volume rendering.
  - Property: Select the current Volume Property. Volume properties contain the opacity, color and gradient transfer functions for each component.
  - View: Select the 3D views where the volume rendering must be displayed into. If no view is selected, the volume rendering is visible in all views
- **Display:** Main properties for the volume rendering.
  - Preset: Apply a pre-defined set of functions for the opacity, color and gradient transfer functions. The generic presets have been tuned for a combination of modalities and organs. They may need some manual tuning to fit your data.
  - Shift: Move all the inner points (first and last excluded) of the current transfer functions to the right/left (lower/higher). It can be useful when a preset defines a ramp from 0 to 200 but your data requires a ramp from 1000 to 1200.
  - Crop: Simple controls for the cropping box (ROI). More controls are available in the "Advanced..." section. Enable/Disable cropping of the volume. Show/Hide the cropping box. Reset the box ROI to the volume's bounds.
  - Rendering: Select a volume rendering method. A default method can be set in the application settings Volume Rendering panel.
    - VTK CPU Ray Casting (default): Available on all computers, regardless of capabilities of graphics hardware. The volume rendering is enterily realized on the CPU, therefore it is slower than other options.
    - VTK GPU Ray Casting: Uses graphics hardware for rendering, typically much faster than CPU volume rendering. This is the recommended method for computers that have sufficiant graphics capabilities. It supports surface smoothing to remove staircase artifacts.
    - VTK Multi-Volume: Uses graphics hardware for rendering. Can render multiple overlapping volumes. Currently does not support cropping.
- Advanced: More controls to control the volume rendering. Contains 3 tabs: "Techniques", "Volume Properties" and "ROI"
  - Techniques: Advanced properties of the current volume rendering method.
    - Quality:
      - Adaptive (default): quality is reduced while interacting with the view (rotating, changing volume rendering settings, etc.).
        - Interactive speed: Ensure the given frame per second (FPS) is enforced in the views during interaction. The higher the FPS, the lower the resolution of the volume rendering
      - Normal: fixed rendering quality, should work well for volumes that the renderer can handle without difficulties.
      - Maximum: oversamples the image to achieve higher image quality, at the cost of slowing down the rendering.
    - Technique:
      - Composite with shading (default): display as a shaded surface
      - Maximum intensity projection: display brightest voxel value encountered in each projection line
      - Minimum intensity projection: display darkest voxel value encountered in each projection line
    - Surface smoothing: check this checkbox to reduce staircase artifacts by adding a random noise pattern (jitter) to the raycasting lines
  - Volume Properties: Advanced views of the transfer functions.
    - Synchronize with Volumes module: show volume rendering with the same color mapping that is used in slice views
      - Click: Apply once the properties (window/level, threshold, lut) of the Volumes module to the Volume Rendering module.
      - Checkbox: By clicking on the checkbox, you can toggle the button. When toggled, any modification occuring in the Volumes module is continuously applied to the volume rendering
    - Control point properties: X = scalar value, O = opacity, M = mid-point, S = sharpness
    - Keyboard/mouse shortcuts:
      - Left button click: Set current point or create a new point if no point is under the mouse.
      - Left button move: Move the current or selected points if any.
      - Right button click: Select/unselect point. Selected points can be moved at once
      - Right button move: Define an area to select points:
      - Middle button click : Delete point under the mouse cursor.
      - Right/Left arrow keys: Change of current point
      - Delete key: Delete the current point and set the next point as current
      - Backspace key : Delete the current point and set the previous point as current
      - ESC key: Unselect all points.
    - Scalar Opacity Mapping: Opacity transfer function. Threshold mode: enabling threshold controls the transfer function using range sliders in addition to control points.
    - Scalar Color Mapping: Color transfer function.
    - Gradient Opacity: Gradient opacity transfer function. This controls the opacity according to how large a density gradient next to the voxel is.
    - Advanced:
      - Interpolation: Linear (default for scalar volumes) or nearest neighboor (default for labelmaps) interpolation.
      - Shade: Enable/Disable shading. Shading uses light and material properties. Disable it to display X-ray-like projection.
      - Material: Material properties of the volume to compute shading effect.
  - ROI: More controls for the cropping box.
    - Display Clipping box: Show hide the bounds of the ROI box.
    - Interactive mode: Control wether the cropping box is instantaneously updated when dragging the sliders or only when the mouse button is released.

## Contributors

- Julien Finet (Kitware)
- Alex Yarmarkovich (Isomics)
- Csaba Pinter (PerkLab, Queen's University)
- Andras Lasso (PerkLab, Queen's University)
- Yanling Liu (SAIC-Frederick, NCI-Frederick)
- Andreas Freudling (SPL, BWH)
- Ron Kikinis (SPL, BWH)

## Acknowledgements

 This work is part of the National Alliance for Medical Image Computing (NAMIC), funded by the National Institutes of Health through the NIH Roadmap for Medical Research, Grant U54 EB005149. Some of the transfer functions were contributed by Kitware Inc. (VolView)

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_kitware.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_isomics.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_spl.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_namic.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_perklab.png)
