# Models

## Overview

This module is used for changing the appearance of and organizing 3d surface models.

## Use cases

### Visualize hierarchy of models

Models can be organized into folders and display properties (visibility, color, opacity) can be overridden for an entire branch.

Folder nodes can be created by right-clicking on a folder and choosing "Create child folder".

### Edit models

Models can be edited using Surface toolbox or Dynamic modeler module, or by converting to segmentation and [editing with Segment Editor module](segmentations.md#editing-a-segmentation-imported-from-model-surface-mesh-file).

## Panels and their use

- **Model tree**:
  - **Filter by name...**: Enter a model name to filter the model hierarchy tree to items that have matching name.
  - **Hide All Models**: Turn off the visibility flag on all models in the scene. If any hierarchies have the "Force color to children" checkbox selected, the hierarchy node visibility will override model visibility settings.
  - **Show All Models**: Turn on the visibility flag on all models in the scene. If any hierarchies have the "Force color to children" checkbox selected, the hierarchy node visibility will override model visibility settings.
  - **Scene tree view**: The tree view of all models and model hierarchies in the current MRML scene.
    - Show/hide models by clicking the eye icon.
    - Change selected display properties by right-clicking on the eye icon.
    - Change color by double-clicking on the color square.
    - Organize models into a hierarchy: Create child folders by right-clicking on a folder item and drag-and-drop models into them. Visibility and opacity specified for a folder is applied to all children. Color specified for a folder is applied to all children if "Apply color to all children" option is enabled (in the right-click menu of the eye icon).

- **Information Panel**:
  - **Information**: Information about this surface model
  - **Surface Area**: The calculated surface area of the model, in square millimeters
  - **Volume**: The volume inside the surface model, in cubic millimeters
  - **Number of Points**: Number of vertices in the surface model
  - **Number of Cells**: Number of cells in the surface model
  - **Number of Point Scalars**: Shows how many arrays of scalars are associated with the points of the surface model.
  - **Number of Cell Scalars**: Shows how many arrays of scalars are associated with the cells of the surface model.
  - **Filename**: Path to the file from which this surface model was loaded and/or where it will be saved by default.

- **Display**: Control the display properties of the currently selected model in the Scene.
  - **Visibility**: Control the visibility of the model.
    - **Visibility**: The model is visible in views if this is checked.
    - **Opacity**: Control the opacity of the model. 1.0 is totally opaque and 0.0 is completely transparent. 1.0 by default.
    - **View**: Specifies which views this modelis visible in. If none are checked, the model is visible in all 2D and 3D views.
    - **Color**: Control the color of the model. Note that the lighting can alter the color. Gray by default.
- **3D Display**:
    - **Representation**: Control the surface geometry representation (points, wireframe, surface, or surface with edges).
    - **Visible sides**: This option can be used to only show front-facing elements, which may make rendering slightly faster, but the inside of the model will be no longer visible (when the viewpoint is inside the model or when the model is clipped). when it is clipped). Showing of backface elements only allows seeing inside the model.
    - **Clipping**: Enable clipped display of the model. Slice planes are used as clipping planes, according to options defined in the "Clipping planes" section at the bottom.
    - **Advanced**
      - **Point Size**: Set the diameter of the model points (if the model is a point cloud or if the representation is "Points".) The size is expressed in screen units. 1.0 by default.
      - **Line Width**: Set the width of the model lines (if the model is a polyline or representation is "Wireframe"). The width is expressed in screen units. 1.0 by default.
      - **Backface Color Offset**: Control the color of the inside of the model (which is visible when the model is open or the viewpoint is inside the model). The values correspond to hue, saturation, and lightness offset compared to the base color.
    - **Edge Color**: Control the color of the model edges (if Edge Visibility is enabled). Black by default.
    - **Lighting**: Control whether the model representation is impacted by the frontfacing light. If enabled, Ambient, Diffuse and Specular parameters are used to compute the lighting effect. Enabled by default.
    - **Interpolation**: Control the shading interpolation method (Flat, Gouraud, Phong) for the model. Gouraud by default. Gouraud and Phong shading requires surface normals. If surface normals are missing then the model will be displayed with flat shading (faceted appearance). Surface Toolbox module can compute normals for a model.
    - Material Properties: Material properties of the currently selected model
      - **Ambient**: Control the constant brightness of the model.
      - **Diffuse**: Control the amount of light that is scattered back from the model. This is direction-dependent: regions that are orthogonal to the view direction appear brighter.
      - **Specular**: Control specular reflection ("shininess") of the model surface.
      - **Power**: The specular power.
      - **Preview**: A rendering of a sphere using the current material properties.
- **Slice Display**:
    - **Visibility**: Control visibility of the model in slice views.
    - **Opacity**: Opacity of the model in slice views.
    - **Mode**: Intersection shows intersection lines of the model with the slice plane. Projection projects the entire model into the slice plane. Distance encoded projection makes the projected model colored based on the distance from the slice plane.
    - **Line width**: Width in pixels of the intersection lines of the model with the slice planes.
    - **Color table**: Specifies colors for "Distance encoded projection" mode.
  - **Scalars**: Scalar overlay properties
    - **Visible**: Color the model using the active scalar.
    - **Active Scalar**: A drop down menu listing the current scalar overlays associated with this model and shows which one is currently active. Most models will have normals, Freesurfer surface models can have multiple scalar overlay files associated with them (e.g., lh.sulc, lh.curv).
    - **Color Table**: Select a color look up table used to map the scalar overlay's values  (usually in the range of 0.0 to 1.0) to colors. There are built in color maps that can be browsed in the Colors module. The most commonly used color map for FreeSurfer scalar overlays is the GreenRed one. Legacy color maps from Slicer2 include Grey, Iron, Rainbow, etc. Those color maps with "labels" in their names are usually discrete and don't work well for the continuous scalar overlay ranges.
    - **Scalar Range Type**: Select which scalar range to use:
      - Manual: range is set manually
      - Data scalar range: range is set to the value range of the active scalar
      - Color table: use range specified in the color table. Useful for showing several nodes using the same color mapping.
      - Data type: range is set to the possible range of the active scalar's data type. This is only useful mostly for 8-bit scalars.
      - Direct color mapping: if active scalar has 3 or 4 components then those are interpreted as RGB or RGBA values.
    - **Displayed Range**: currently used scalar range.
    - **Threshold**: if enabled, then regions of the model that are outside the specified range will be hidden.
  - **Clipping Planes**:
    - **Clip selected model**: enable clipping for the selected model.
    - **Clipping Type**: When more than one slice plane is used, this option controls if it's the union or intersection of the positive and/or negative spaces that is used to clip the model. The parts of the model inside the selected space is kept, parts outside of the selection are clipped away.
    - **Red/Yellow/Green Slice Clipping**: Use the positive or negative space defined by the Red/Yellow/Green slice plane to clip the model. Positive side is toward the Superior/Right/Anterior direction. Keeps the part of the model in the selected space, clips away the rest.
  - **Clip only whole cells when clipping**: choose between straight cut (cuts over cells) or crinkle cut (clips the model along cell boundaries).

## Contributors

Julien Finet (Kitware), Alex Yarmarkovich (Isomics), Nicole Aucoin (SPL, BWH)

## Acknowledgements

This work is part of the National Alliance for Medical Image Computing (NAMIC), funded by the National Institutes of Health through the NIH Roadmap for Medical Research, Grant U54 EB005149. This work is partially supported by the Air Force Research Laboratories (AFRL).
