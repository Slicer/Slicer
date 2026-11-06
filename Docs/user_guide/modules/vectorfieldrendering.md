# Vector Field Rendering

## Overview

This module displays vector fields as glyphs, a deformed grid, contours of the magnitude, or streamlines, in slice and 3D views. The vector field can be:

- a 3-component point data or cell data array of a model, for example flow velocity or displacement vectors at the points of a mesh,
- a vector volume, which stores a vector in each voxel, for example a displacement or velocity field,
- a transform: its displacement vectors.

A node can have any number of vector field displays, each drawn on its own: for example glyphs and streamlines of the same field at the same time.

## Use cases

- Show the velocity field of a flow simulation (stored in a model) or of a 4D flow MRI (stored in a vector volume) as glyphs or streamlines.
- Show the displacement of a registration transform as glyphs, a deformed grid, or contours of the displacement magnitude.

## How to

### Show a vector field

1. Select the model, vector volume or transform in the tree at the top of the module.
2. Click **Add** in the **Vector field displays** section.
3. Choose the visualization mode and settings in the **Display** section.

A transform is defined everywhere, so in 3D views it is only drawn after a node is selected in **Sample at** (for example a markups ROI), which tells where to draw it. Slice views draw it in the slice plane without that.

### Show or hide a vector field from the Data module

Right-click the eye icon of a model, vector volume or transform in the subject hierarchy tree (for example in the Data module) and check **Show as vector field**. A vector field display is added if the node has none. While this is checked, the eye icon shows and hides the vector field together with the node. When it is unchecked, the vector field stays hidden when the eye icon shows the node again. **Vector field display settings...** opens this module with the node selected.

## Panels and their use

- **Node tree**: models, vector volumes and transforms in the scene. The selected node is the one whose vector field displays are listed below. The search box above the tree shows only the nodes whose name contains the text.
- **Vector field displays**: the vector field displays of the selected node. **Add** adds a display, **Remove** removes the selected one. Unchecking a display hides it, and it stays hidden when the node is shown by its eye icon; checking it shows it again. If the selected node has no vector field (a model without a 3-component array, a volume with less than 3 components) then a note says so and no display can be added.
- **Display**: settings of the display that is selected in the list.
  - **Vector array**: Array that contains the vectors. Models only: all 3-component point data and cell data arrays of the model are listed. Vectors stored in a cell data array are drawn at the cell centers (one glyph per cell). When an array is chosen, the scale factor is set so that the longest vector is about 1/20 of the size of the displayed region.
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
  - **Sample at**: Node that the samples are taken from: glyphs are placed at them, streamlines start from them, and grids and contours are built over it. A point list, a curve, a plane, a markups ROI, a slice view, a model, a segmentation, or a volume. "(This model)" or "(This volume)" samples the node itself; streamlines then start from its points (or a random subset of them). Only shown when the field can be evaluated anywhere (the model has volumetric cells, or **Interpolate between points** is enabled); otherwise glyphs are always drawn at the points of the model. Not shown in contour mode for a model with volumetric cells (without **Interpolate between points**): its contours are computed exactly on its cells, over the whole mesh, so nothing is sampled (use **Mask** to limit where they are shown).
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
    - **Color legend**: shows a color legend for the colors of the field in the views, The legend is added when the section is opened for the first time, hidden until **Visible** is checked.
    - **Color map**: for a color map that maps values to colors (such as the default "Displacement to color" map of a transform, which maps displacement in mm to color), the colors can be edited here. Click on a small circle to see its value and color above the color bar, drag the circles or edit the values, click on the color bar to add a color, and press the DEL key to delete the selected color.

### Differences by the kind of node

- **Model**: the vector array is chosen in **Vector array**. A surface mesh or point cloud only has values at its points: grid, contour and streamline modes need **Interpolate between points**, or a mesh with volumetric cells.
- **Vector volume**: the field is always the voxel vectors, colored by magnitude by default with the Plasma color table. The vector components are read as RAS directions.
- **Transform**: the field is the displacement of the transform, in mm. Streamline mode is not available. By default it is colored by the displacement magnitude with the "Displacement to color" color map: 1 mm (or below) gray, 2 mm green, 5 mm yellow, 10 mm (or above) red. Transforms that were displayed in scenes saved by earlier Slicer versions are shown the same way when the scene is loaded.

## Related modules

- [Models](models.md), [Volumes](volumes.md), [Transforms](transforms.md): the nodes whose vector fields are shown.
- [Colors](colors.md): color tables and color legends.

## Information for developers

The display node (`vtkMRMLVectorFieldDisplayNode`), its processing pipeline and the displayable managers are described in the [Vector field display](../../developer_guide/advanced_topics.md#vector-field-display) section of the developer guide. Display nodes can be added from Python using the module logic:

```python
volumeNode = getNode("Velocity")
displayNode = slicer.vtkSlicerVectorFieldRenderingLogic.AddVectorFieldDisplayNode(volumeNode)
displayNode.SetVisualizationMode(slicer.vtkMRMLVectorFieldDisplayNode.VisualizationModeStreamline)
```

## Contributors

Andras Lasso (PerkLab, Queen's University)

## Acknowledgements

This module was developed with funding from the Children's Hospital of Philadelphia Frontier Program.
