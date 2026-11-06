# Models

## Overview

This module is used for changing the appearance of and organizing 3d surface models, and for displaying vector fields stored in models.

## Use cases

### Visualize hierarchy of models

Models can be organized into folders and display properties (visibility, color, opacity) can be overridden for an entire branch.

Folder nodes can be created by right-clicking on a folder and choosing "Create child folder".

### Visualize vector field

If a model contains vectors (such as flow velocity or displacement vectors, stored in a 3-component point data or cell data array) then they can be visualized as glyphs, deformed grid, contours, or streamlines in Models module's Display / Vector field section. See details in the [Vector field](models.md#panels-and-their-use) section description.

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
    - **Merge coincident points**: Only shown for volumetric meshes (unstructured grids, such as tetrahedral meshes). The surface of a volumetric mesh is displayed by showing the cell faces that are not shared with neighbor cells. If neighbor cells do not share points (for example, along boundaries between mesh partitions that a simulation software exported separately) then their common faces are displayed as internal surfaces, which are visible when the model is semi-transparent. If this option is enabled then points that have exactly the same position are merged for display, which removes these internal surfaces. Point data of merged points is taken from one of them. The model data is not changed. Enabled by default.
- **Slice Display**:
    - **Visibility**: Control visibility of the model in slice views.
    - **Opacity**: Opacity of the model in slice views.
    - **Mode**: Intersection shows intersection lines of the model with the slice plane. Projection projects the entire model into the slice plane. Distance encoded projection makes the projected model colored based on the distance from the slice plane.
    - **Line width**: Width in pixels of the intersection lines of the model with the slice planes.
    - **Color table**: Specifies colors for "Distance encoded projection" mode.
  - **Scalars**: Scalar overlay properties
    - **Visible**: Color the model using the active scalar.
    - **Active Scalar**: A drop down menu listing the current scalar overlays associated with this model and shows which one is currently active. Most models will have normals, Freesurfer surface models can have multiple scalar overlay files associated with them (e.g., lh.sulc, lh.curv). For an array with more than one component (such as a vector array), the box next to it chooses what is shown: **Magnitude** (the length of the vectors, default) or one component (0, 1, 2, ...). The displayed range and the threshold use the same values.
    - **Color Table**: Select a color look up table used to map the scalar overlay's values  (usually in the range of 0.0 to 1.0) to colors. There are built in color maps that can be browsed in the Colors module. The most commonly used color map for FreeSurfer scalar overlays is the GreenRed one. Legacy color maps from Slicer2 include Grey, Iron, Rainbow, etc. Those color maps with "labels" in their names are usually discrete and don't work well for the continuous scalar overlay ranges.
    - **Scalar Range Type**: Select which scalar range to use:
      - Manual: range is set manually
      - Data scalar range: range is set to the value range of the active scalar
      - Color table: use range specified in the color table. Useful for showing several nodes using the same color mapping.
      - Data type: range is set to the possible range of the active scalar's data type. This is only useful mostly for 8-bit scalars.
      - Direct color mapping: if active scalar has 3 or 4 components then those are interpreted as RGB or RGBA values.
    - **Displayed Range**: currently used scalar range.
    - **Threshold**: if enabled, then regions of the model that are outside the specified range will be hidden.
  - **Vector field**: Display a vector field stored in the model (for example, velocity or displacement vectors at mesh points) as glyphs, deformed grid, contours, or streamlines. A model can have multiple vector field displays, for example to show the same field as glyphs and as streamlines at the same time.
    - **Vector field display list**: Vector field displays of the model. Click **+** to add a new display, **-** to remove the selected display. The settings below apply to the display that is selected in the list.
    - **Vector array**: Array that contains the vectors. All 3-component point data and cell data arrays of the model are listed. If the model has no such array then there is nothing to display, a note says so, and the settings are disabled. Vectors stored in a cell data array are drawn at the cell centers (one glyph per cell). When an array is chosen, the scale factor is set so that the longest vector is about 1/20 of the size of the displayed region.
    - **Visibility**: Show the vector field in the views.
    - **Visibility in 3D view**: Show the vector field in 3D views.
    - **Visibility in slice view**: Show the vector field in slice views.
    - **Mode**: How the vector field is drawn.
      - **Glyphs**: a glyph (arrow, cone, sphere, etc.) at each sampling point, oriented and scaled by the vector. In slice views flat glyphs are drawn, which show the component of the vector that lies in the slice plane; they are colored by the magnitude of the full 3D vector, so colors match the 3D views.
      - **Grid**: a regular lattice of lines, deformed by the vectors.
      - **Contour**: iso-surfaces (in 3D views) or iso-lines (in slice views) of the vector magnitude.
      - **Streamlines**: curves that follow the direction of the field, started from seed points. In slice views the streamlines are traced in 3D and the part that is within the slice thickness is shown.
      - Glyph mode is always available. Grid, contour, and streamline modes require that the field can be evaluated at any position: the model must have volumetric (3D) cells, such as a tetrahedral mesh, or **Interpolate between points** must be enabled.
    - **Interpolate between points**: If enabled, a smooth field (cubic B-spline, computed using multilevel B-spline approximation) is fitted to the vectors at the mesh points (or cell centers, for cell data), so that the field can be evaluated anywhere. This makes all visualization modes available in both 3D and slice views, and allows placing glyphs on a regular lattice or at points of another node. The fit is recomputed when the mesh or the array changes. Surface meshes and point clouds need this option for anything but glyphs at their points. Models with volumetric cells are interpolated exactly inside their cells without it, but only there; the fit extends the field beyond the mesh (up to **Maximum distance from points**), for example to show a deformed grid or streamlines around a thin solid.
      - **Maximum distance from points**: The interpolated field is not drawn at positions that are farther than this distance (in mm) from all the data points, so that nothing is shown in regions where there is no data. 0 means automatic (10% of the diagonal of the bounding box of the data).
      - **Fit error at points**: The largest difference between the fitted field and the input vectors at the input points. The fitted field approximates the data (it does not exactly reproduce the input vectors), and this value shows how close the approximation is.
    - **Sample at**: Node that the samples are taken from: glyphs are placed at them, streamlines start from them, and grids and contours are built over it. A point list, a curve, a plane, a markups ROI, a slice view, a model, a segmentation, or a volume. "(This model)" samples the model itself; streamlines then start from its points (or a random subset of them). Only shown when the field can be evaluated anywhere (the model has volumetric cells, or **Interpolate between points** is enabled); otherwise glyphs are always drawn at the points of the model. Not shown in contour mode for a model with volumetric cells (without **Interpolate between points**): its contours are computed exactly on its cells, over the whole mesh, so nothing is sampled (use **Mask** to limit where they are shown).
    - **Sampling**: Which part of the node is sampled: its **Points**, its **Surface**, or its **Inside** (only the parts that the node has are offered), and how: **Regular** (every point, points about **Spacing** apart on the surface of a model or segmentation, or a lattice or steps along a curve **Spacing** apart; the same spacing is the resolution of grids and contours) or **Random** (**Number of points** samples spread evenly at random). For the model itself, a random sample is a subset of its points spread evenly over its bounds, surface, or volume. Grids and contours are built on a lattice, over the inside of the node or in its plane, or over the whole field, except contours of a model with volumetric cells, which are built on its cells (the sampling options and **Spacing** are then hidden).
    - **Mask**: Only show the field inside (or outside) of this node, in every mode and every view: a markups ROI, a closed model, a segmentation, or the extent of a volume. Glyphs outside are not drawn; grid lines and contours are cut where they leave it; streamlines are only integrated where the field is shown: they stop where they leave it, and no streamline starts where it is hidden. **Segments**: for a segmentation, the segments that mask the field (all of them if none is checked).
    - **Scale factor**: A slider that scales the drawn vectors (the glyph length in glyph mode, the deformation in grid mode). With a value of 1 the vectors are drawn at their actual length. **Reverse** (next to the slider) draws the vectors pointing the opposite way (in grid mode: deforms the grid the opposite way); it is stored as a negative scale factor.
    - **Threshold**: If enabled, the field is only drawn where the colored value (the vector magnitude, or the chosen component) is inside the specified range. Streamlines are only integrated there, as with the mask: they stop where the value leaves the range, and no streamline starts where it is out of the range. Not available in contour mode.
    - **Advanced**: Settings of the selected visualization mode.
      - Glyph mode:
        - **Glyph type**: Shape of the glyphs: arrow, cone, box, cylinder, line, or sphere.
        - **Orientation array**, **Scale array**: Arrays that orient and scale the glyphs. "(Default)" uses the selected vector array. If scale array is set to "None" then all glyphs have the same size (only scaled by the scale factor).
        - **Vector scale mode**: If the scale array has 3 components: scale glyphs uniformly by the vector magnitude, or scale each axis by the corresponding vector component.
      - Grid mode: **Grid spacing** (distance between grid lines), **Non-warped grid visibility** (show the undeformed grid too, in 3D and slice views, in the color chosen next to the checkbox), **Grid lines visibility** (draw the deformed grid lines; turn it off to show only the fill or the undeformed grid), **Fill** (fill the surface of the grid with the colors of the field: with a plane or slice as **Sample at**, a colored map of the field in that plane, flat when the scale factor is 0; in slice views the slice plane is filled).
      - Contour mode: **Contour levels**: magnitude values, separated by spaces, where the iso-surfaces/iso-lines are drawn. If empty, then a few levels are drawn, spread over the color range.
      - Streamline mode:
        - **Bidirectional**: Trace streamlines in both directions from the seed points, not just in the direction of the field.
        - **Initial step**: Integration step size at the start. Smaller value follows sharply turning fields more accurately but makes computation slower.
        - **Maximum length**: Maximum length of a streamline.
        - **Streamline diameter**: Diameter of the streamline tubes. 0 draws thin lines.
      - **3D display**: **Opacity**, **Grid line diameter**, **Glyph diameter** (glyph thickness; in mm if **absolute** is pressed, otherwise as a percentage of the glyph length), and glyph geometry (**Shaft diameter**, **Tip length**, **Resolution**).
      - **Slice display**: **Opacity** of everything drawn in slice views (glyphs, grid lines and fill, isolines, streamlines), **Line width**, and geometry of the flat glyphs drawn in slice views (**Tip length**, **Resolution**).
    - **Coloring**:
      - **Color by**: Draw the field with a single color (**Solid color**, specified by **Color**) or color it by the vector **Magnitude**. When coloring by magnitude, the array, color table, and range used for coloring can be specified the same way as in the Scalars section, including the component of a multi-component array: **Magnitude** or one component of the vectors (for example 0 for the R component of a displacement in RAS).
      - **Color legend**: shows a color legend for the colors of the field in the views, the same way as the Color Legend of the Scalars section does for the surface. The legend is added when the section is opened for the first time, hidden until **Visible** is checked.
  - **Clipping Planes**:
    - **Clip selected model**: enable clipping for the selected model.
    - **Clipping Type**: When more than one slice plane is used, this option controls if it's the union or intersection of the positive and/or negative spaces that is used to clip the model. The parts of the model inside the selected space is kept, parts outside of the selection are clipped away.
    - **Red/Yellow/Green Slice Clipping**: Use the positive or negative space defined by the Red/Yellow/Green slice plane to clip the model. Positive side is toward the Superior/Right/Anterior direction. Keeps the part of the model in the selected space, clips away the rest.
  - **Clip only whole cells when clipping**: choose between straight cut (cuts over cells) or crinkle cut (clips the model along cell boundaries).

## Contributors

Julien Finet (Kitware), Alex Yarmarkovich (Isomics), Nicole Aucoin (SPL, BWH)

## Acknowledgements

This work is part of the National Alliance for Medical Image Computing (NAMIC), funded by the National Institutes of Health through the NIH Roadmap for Medical Research, Grant U54 EB005149. This work is partially supported by the Air Force Research Laboratories (AFRL).
