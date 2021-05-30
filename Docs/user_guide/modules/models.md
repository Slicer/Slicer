# Models

## Overview

This module is used for changing the appearance of and organizing 3d surface models.

## Panels and their use

- **Top level**: 
  - **Include Fibers**: Check this box to include fiber bundle nodes in the scene model hierarchy view.
  - **Scroll to...**: Enter a model name to scroll the model hierarchy tree view to the first model matching that name.
  - **Hide All Models**: Turn off the visibility flag on all models in the scene. If any hierarchies have the "Force color to children" checkbox selected, the hierarchy node visibility will override model visibility settings.
  - **Show All Models**: Turn on the visibility flag on all models in the scene. If any hierarchies have the "Force color to children" checkbox selected, the hierarchy node visibility will override model visibility settings.

- **Scene**: 
  - **Scene tree view**: The tree view of all models and model hierarchies in the current MRML scene. You can right click on the Scene element to add hierarchies and drag and drop models into them. Control the Visible property of the model via the eye icon (open/close to show/hide the models). The model color and opacity are also displayed on the same line as the model name. Model hierarchies allow you to group together models. When using the ModelMaker module, multiple models created from one label map are grouped under a model hierarchy node.

- **Information Panel**: 
  - **Information**: Information about this surface model
  - **Surface Area**: The calculated surface area of the model, in square millimetres
  - **Volume**: The volume inside the surface model, in cubed millimetres
  - **Number of Points**: Number of vertices in the surface model
  - **Number of Cells**: Number of cells in the surface model
  - **Number of Point Scalars**: Shows how many arrays of scalars are associated with the points of the surface model.
  - **Number of Cell Scalars**: Shows how many arrays of scalars are associated with the cells of the surface model.
  - **Filename**: Path to the file from which this surface model was loaded and/or where it will be saved.

- **Display**: Control the display properties of the currently selected model in the Scene.
  - **Visibility**: Control the visibility of the model.
    - **Visible**: Is this surface model visible in the 3D view?
    - **View**: In which views is this surface model visible? If none are checked, the model is visible in all 2D and 3D views.
    - **Clip**: Is clipping (using the slice planes) enabled for this model? Once it is on, go to the Clipping pane to set the options.
    - **Slice Intersections Visible**: If this flag is set, the intersections of the model with the slices will be computed and shown as an outline on the slice planes.
    - **Slice Intersections Thickness**: Width in pixels of the intersection lines of the model with the slice planes.
  - **Representation**: Control the representation of the model.
    - **Representation**: Control the surface geometry representation (points, wireframe or surface) for the object. The default is Surface. Note that you can change the representation of all the models to wireframe with the 'w' key shortcut in the 3D view.
    - **Point Size**: Set the diameter of the model points (if the model is a point cloud or if the representation is "Points".) The size is expressed in screen units. 1.0 by default.
    - **Line Width**: Set the width of the model lines (if the model is a polyline or representation is "Wireframe"). The width is expressed in screen units. 1.0 by default.
    - **Frontface culling**: Turn on/off culling of polygons based on the orientation of normals with respect to the camera. If frontface culling is on, polygons facing toward the camera are not drawn. Visible by default.
    - **Backface culling**: Turn on/off culling of polygons based on orientation of normals with respect to the camera. If backface culling is on, polygons facing away from the camera are not drawn. This feature needs to be turned off if the inside of a model is viewed, either when clipping or when moving the camera inside an object. Hidden by default.
  - **Color**: Control the colors of the model.
    - **Color**: Control the color of the model. Note that the lighting can alter the color. Gray by default.
    - **Opacity**: Control the opacity of the model. 1.0 is totally opaque and 0.0 is completely transparent. 1.0 by default.
    - **Edge Visibility**: Turn on/off the visibility of the model edges (the wireframe). Hidden by default
    - **Edge Color**: Control the color of the model edges (if Edge Visibility is enabled). Black by default.
  - **Lighting**: Control the lighting effects on the model.
    - **Lighting**: Control whether the model representation is impacted by the frontfacing light. If enabled, Ambient, Diffuse and Specular parameters are used to compute the lighting effect. Enabled by default.
    - **Interpolation**: Control the shading interpolation method (Flat, Gouraud, Phong) for the model. Gouraud by default.
    - **Shading**: Control the shading of the model. The material properties must be set if enabled.
  - **Material Properties**: Material properties of the currently selected model
    - **Ambient**: Controls the base lighting for the model.
    - **Diffuse**: Controls the amount of light scattered from the model.
    - **Specular**: Controls the highlights on the model.
    - **Power**: The specular power.
    - **Preview**: A rendering of a sphere using the current material properties, useful if the surface model isn't regular.
  - **Scalars**: Scalar overlay properties
    - **Visible**: Are the scalars visible on the surface model?
    - **Active Scalar**: A drop down menu listing the current scalar overlays associated with this model and shows which one is currently active. Most models will have normals, Freesurfer surface models can have multiple scalar overlay files associated with them (e.g. lh.sulc, lh.curv).
    - **Active Scalar Range**: Displays the numerical range spanned by the currently active scalar array (used when the Data scalar range type is selected).
    - **Color Table**: Select a color look up table used to map the scalar overlay's values  (usually in the range of 0.0 to 1.0) to colors. There are built in color maps that can be browsed in the Colors module. The most commonly used color map for FreeSurfer scalar overlays is the GreenRed one. Legacy color maps from Slicer2 include Grey, Iron, Rainbow, etc. Those color maps with "labels" in their names are usually discrete and don't work well for the continuous scalar overlay ranges.
    - **Scalar Range Type**: Select which scalar range to use: range in the scalar data array, Color Table range, Display Scalar Range, range of the data type of the scalar array.
    - **Display Scalar Range**: Set the scalar range on the model's display node.
  - **Clipping Panel**: 
    - **Clipping**: Control the clipping properties for this surface model
    - **Clipping Type**: When more than one slice plane is used, this option controls if it's the union or intersection of the positive and/or negative spaces that is used to clip the model. The parts of the model inside the selected space is kept, parts outside of the selection are clipped away.
    - **Red Slice Clipping**: Use the positive or negative space defined by the Red slice plane to clip the model. Positive side is toward the Superior, negative is toward the Inferior. Keeps the part of the model in the selected space, clips away the rest.
    - **Yellow Slice Clipping**: Use the positive or negative space defined by the Yellow slice plane to clip the model. Positive side is toward the Right, negative is toward the Left. Keeps the part of the model in the selected space, clips away the rest.
    - **Green Slice Clipping**: Use the positive or negative space defined by the Green slice plane to clip the model. Positive side is toward the Anterior, negative is toward the Posterior. Keeps the part of the model in the selected space, clips away the rest.

## Contributors

Julien Finet (Kitware), Alex Yarmarkovich (Isomics), Nicole Aucoin (SPL, BWH)

## Acknowledgements

This work is part of the National Alliance for Medical Image Computing (NAMIC), funded by the National Institutes of Health through the NIH Roadmap for Medical Research, Grant U54 EB005149. This work is partially supported by the Air Force Research Laboratories (AFRL).
