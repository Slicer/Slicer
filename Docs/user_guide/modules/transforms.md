# Transforms

## Overview

This module is used for creating, editing, and visualization of spatial transformations. Transformations are stored in transform nodes and define position, orientation, and warping in the world coordinate system or relative to other nodes, such as volumes, models, markups, or other transform nodes.

See a summary of main features demonstrated in [this video](https://screencast.com/t/Z6dQVjK3m).

Supported transform types:
- linear transform: rigid, scaling, shearing, affine, etc. specified by a 4x4 homogeneous transformation matrix
- b-spline transform: displacement field specified at regular grid points, with b-spline interpolation
- grid transform: dense displacement field, with trilinear interpolation
- thin-plate splines: displacements specified at arbitrarily placed points, with thin-plate spline interpolation
- scattered transform: displacements stored as a point data array of a model (for example the displacement of a solid in a simulation), approximated by a smooth b-spline transform. See [Compute transform](transforms.md#compute-transform).
- composite transforms: any combinations of the transforms above, in any order, any of them optionally inverted

## Use cases

### Create a transform

Transform node can be created in multiple ways:
- Method A: In Data module's Subject hierarchy tab, right-click on the "Transform" column and choose "Create new transform". This always creates a general "Transform".
- Method B: In Data module's Subject hierarchy tab, right-click on the "Visibility" column of any transformable node and choose "Interaction" to create a parent transform and display the interaction handles (See [Interaction section](transforms.md#interaction)). This always creates a general "Transform".
- Method C: In Data module's Transform hierarchy tab, right-click on an item and choose "Insert transform". This always creates a "Linear transform". Advantage of this method is that it is easy to build and overview hierarchy of transforms.
- Method D: In Transforms module click on the "Transform" node selector and choose one of the "Create new..." options.

How to choose transform type: Create "Linear transform" if you only work with linear transforms, because certain modules only allow you to select this node type as input. In other cases, it is recommended to  create the general "Transform" node. Multiple transform node types exist because earlier Slicer could only store a simple transformation in a node. Now a transform node can contain any transformation type (linear, grid, bspline, thin-plate spline, or even composite transformation - an arbitrary sequence of any transformations), therefore transform node types only differ in their name. In some modules, node selectors only accept a certain transform node type, therefore it may be necessary to create that expected transform type, but in the long term it is expected that all modules will accept the general "Transform" node type.

### Apply transform to a node

"Applying a transform" means setting parent transform to a node to translate, rotate, and/or warp it. If the parent transform is changed then the position or shape of the node is automatically updated. If the transform is removed then original (non-transformed) state of the node is restored.

A transform can be applied to a node in multiple ways:
- Method A: In Data module's Subject hierarchy tab, right-click on the "Transform" column and choose a transform (or "Create new transform"). The transform can be interactively edited in 3D views by right-clicking on "Transform" column and choosing "Interaction".
- Method B: In Data module's Transform hierarchy tab, drag the nodes under a transform.
- Method C: In Transforms module's "Apply transform" section move nodes from the Transformable list to the Transformed list by selecting them and click the arrow button between them.

Parent transform can be set for transform nodes, thereby creating a hierarchy (tree) of transforms. This transform tree is displayed in Data module's Transform hierarchy tab.

### Harden transform on a node

"Hardening a transform" means permanently modify the node according to the currently applied transform. For example, coordinate values of model points are updated with the transformed coordinates, or a warped image voxels are resampled. After hardening the transform, the node is no longer associated with the transform.

A transform can be hardened on a node in multiple ways:
- Method A: In Data module, right-click on Transform column and click "Harden transform"
- Method B: In Transforms module's "Apply transform" section, click the harden button (below the left arrow button).

If non-linear transform is hardened on a volume then the volume is resampled using the same spacing and axis directions as the original volume (using linear interpolation). Extents are updated to fully contain the transformed volume. To specify a different image extent or resolution, one of the image resampling modules can be used, such as "Resample Scalar/Vector/DWI volume".

### Modify transform

- Invert: all transforms can be inverted by clicking "Invert" button in Transforms module's Edit section. This is a reversible operation, as the transform's internal representation is not changed, just a flag is set that the transform has to be interpreted as its inverse.
- Split: a composite transform can be split to multiple transform nodes (so that each component is stored in a separate transform node) by clicking "Split" button in Transforms module's Edit section.
- Change translation/rotation:
  - linear transforms can be edited using translation and rotation sliders Transforms module's Edit section. "Translation in global or local reference frame" button controls if translation is performed in the parent coordinate system or the rotated coordinate system.
  - translation and rotation of a linear transform can be interactively edited in 3D by enabling "Visibility in 3D view" in Transforms module's Display / Interaction section. See [Interaction section](transforms.md#interaction).
- Edit warping transform: to specify/edit a warping transform that translates a set of points to specified positions, you can use [semi-automatic registration methods](../registration.md#semi-automatic-registration)

### Compute transform

Transforms are usually computed using [spatial registration tools](../registration.md).

A transform can also be computed from displacements stored in a model, for example the displacement of a solid computed by a simulation, so that the displacement can be applied to other models or images, or displayed outside the solid. Such a scattered transform is an ordinary transform node whose transform is computed from a 3-component point data array of the mesh of a model (its "displacement model"):

- `vtkScatteredBSplineTransform` (vtkAddon): a smooth cubic b-spline is fitted to the displacements at the mesh points (multilevel b-spline approximation). It is defined everywhere: between the points, and beyond them within a margin, where it follows the overall linear trend of the displacements.
- `vtkMeshDisplacementTransform`: the displacements are used as they are, nothing is fitted. The points of the mesh are moved by their own vectors (so the model itself, placed under the transform, is warped exactly as by its vectors), and any other position by the vector interpolated in the cell of the mesh that contains it. Positions outside the cells are not moved, so this is meant for volumetric meshes, or for warping the model itself.

The transform follows the model: it is computed again when the mesh changes (also when another mesh is set, for example along a sequence), and everything transformed by it is updated. It is not saved in a file of its own: the displacement model and the parameters are saved in the scene, and the transform is computed again when the scene is loaded.

To create one, select (or create) a transform in the Transforms module, and in the "Convert" section choose "Import", "Model", the model and the displacement array. The model, the array and the parameters can then be changed in the Edit section. From the Python console:

```python
modelNode = getNode("solid")
transformNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLTransformNode")
slicer.modules.transforms.logic().ImportTransformFromModel(transformNode, modelNode, "Displacement")
transform = transformNode.GetScatteredTransform()  # a vtkScatteredBSplineTransform
transform.Update()
print("Fit error [mm]:", transform.GetResidual())
# Warp another model, or an image, by the displacements
getNode("otherModel").SetAndObserveTransformNodeID(transformNode.GetID())
```

`ImportTransformFromModel(..., keepLinked=False)` instead sets a copy of the fit, an ordinary b-spline transform that does not follow later changes of the model and can be saved to a file.

Parameters (in the Edit section, "Displacement scale" and the "Fitting" group; the values are set from the points when the transform is created):

| Parameter | GUI | Meaning | Default |
|---|---|---|---|
| transform class | Interpolation | `vtkScatteredBSplineTransform` ("B-spline fit") or `vtkMeshDisplacementTransform` ("Mesh"), see above. The parameters below, except `DisplacementScale`, only apply to the b-spline fit. | B-spline fit |
| `DisplacementScale` | Displacement scale | multiplies the displacements: 0 is the identity, 2 twice the fitted displacements; for exaggerating or animating a deformation. **Reverse** (next to the slider) makes it negative, moving points the opposite way. The fit is not affected. | 1 |
| `GridBoundsMargin` | Extrapolation distance | how far beyond the points the transform is fitted, in mm | a tenth of the diagonal of the bounding box of the points |
| `ControlPointSpacing` | Coarsest spacing | b-spline control point spacing of the coarsest level, along each axis, in mm; each further level halves it | 0 = a tenth of the size of the fitted region along that axis |
| `MinimumControlPointSpacing` | Finest spacing | refinement stops before the spacing gets smaller than this, in mm | a hundredth of the size of the fitted region |
| `MaximumNumberOfLevels` | Maximum levels | largest number of refinement levels (1-16); each level needs about 8 times as much memory | 8 |
| `Tolerance` | Tolerance | refinement stops when the largest error at the points is below this, in mm | a thousandth of the largest displacement |
| `UseLinearApproximation` | Linear approximation | start from the best-fitting linear transform, so that the transform follows the linear trend of the displacements away from the points instead of flattening out | on |
| `BorderMode` | Border mode | beyond the fitted region: keep the displacement at the border (`Edge`), no displacement (`Zero`), or fade to zero at the border (`ZeroAtBorder`) | `Edge` |

The Edit section also shows the fit: the largest difference between the transform and the displacements at the points (`GetResidual()`), and the number of refinement levels it used (`GetNumberOfLevels()`). The transform can be hardened, or exported to a grid transform in the "Convert" section, to save it in a file.

### Save transform

Transforms are saved into [ITK](https://itk.org/) transform file format. ITK always saves transform "from parent", as this is the direction that is necessary for transforming images.

If a transform node in Slicer has a transformation with "to parent" direction (e.g., it was computed like that or a "from parent" transform got inverted) then:
- linear transforms: the transform is automatically converted to "from parent" direction.
- warping transforms: the transform is saved as special inverse transform class that most ITK-based applications cannot interpret. If compatibility with other applications is needed, the transform can be converted to a displacement field before saving.

A quick way to export a linear transform to another software or to text files is to copy the homogeneous transformation matrix values to the clipboard by clicking "Copy" button in Edit section in Transforms module.

### Load transform

Drag-and-drop the transform file to the application window and make sure "Transform" is selected in Description column.

MetaImage (mha), NIFTI (nii) vector volumes can be loaded as displacement field (grid) transform. The volume and its vectors are expected to be stored in LPS coordinate system (during reading they are converted to RAS to match coordinate system conventions in Slicer).

Sequences of displacement field (grid) transforms can be loaded from and saved to NRRD sequence (.seq.nrrd) and NIfTI (.nii, .nii.gz) files. See [supported transform file formats](../data_loading_and_saving.md#transforms) for details.

A quick way to import a linear transform from another software or from text files is to copy the homogeneous transformation matrix values to the clipboard, and click "Paste" button in Edit section in Transforms module.

### Visualize transform

Transforms can be visualized in both 2D and 3D views, as glyphs representing the displacement vectors as arrows, cones, or spheres; regular grids that are deformed by the transform; or contours that represent lines or surfaces where the displacement magnitude has a specific value. See documentation of [Display section](transforms.md#display) for details. Linear transforms can also be interacted with using an interaction handle widget. See [Interaction section](transforms.md#interaction)

### Interaction

![Interaction handles in 2D and 3D views.](https://github.com/Slicer/Slicer/releases/download/docs-resources/interaction_handles_1.gif)

#### Interaction keybinds

For macOS, refer to [alternate macOS keybindings](../user_interface.md#alternate-macos-keybindings).

| Handle | Key | Operation |
| -------- | --------- | --------- |
| Translation | `Left-click` | Translate along a specific axis, or parallel to the view plane |
| Translation |  `Left-click` + `Alt` | Translate the center of transformation |
| Rotation |  `Left-click` | Rotate around a specific axis, or parallel to the view plane |
| Scale |  `Left-click` | Scale along a specific axis |
| Scale |  `Left-click` + `Alt` | Uniform scale along a specific axis |
| Translation |  `Left-click` + `Alt` | Translate the center of transformation |
| Any | `Right-click` | Show context menu
| Any | `Right-click` during interaction | Cancel and return to original state |
| Any | `Escape` during interaction | Cancel and return to original state |

:::{note}
In slice views, if "Enable view plane translation anywhere" is enabled (Display section / Interaction / More options), left-click and drag anywhere in the slice view adjusts the translation, instead of only when clicking precisely on the small handle at the center of the transform. While this option is enabled, that handle is drawn as a crosshair (with an empty center) instead of a filled circle. Dragging this crosshair modifies the position of the center of rotation.
:::

#### Context menu

![Context menu showing the following options when right-clicking on a transform node.](https://github.com/Slicer/Slicer/releases/download/docs-resources/interaction_context_menu_1.png)

- Edit properties: Switch to the Transforms module and select the specified transform node.
- Invert transform: Invert the transform matrix.
- Reset transform to identity: Reset the transform matrix back to an identity transform.
- Reset center of transformation:
  - Transform origin: Reset the center of transformation to [0,0,0] in the local coordinate system.
  - Transformed nodes: Reset the center of transformation to the bounds center of all nodes that are transformed.
  - Individual nodes: Reset the center of transformation to the center of the bounds for a specific node.

## Panels and their use

Transform: Select the transform node to display, control and edit.

### Information

Displays details about what transformation(s) a transform node contains, such as type and direction ("to parent" or "from parent).

It also displays displacement value at the current mouse pointer position (in slice views, it is enough to just move the mouse pointer; in 3D views, Shift key must be held down while moving the mouse).

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_transforms_information.png)

### Edit

- Transform matrix: 4x4 homogeneous transformation matrix. Each element is editable on double click. Type Enter to validate change, Escape to cancel or Tab to edit the next element. First 3 columns of the matrix specifies an axis of the transformed coordinate system. Scale factor along an axis is the column norm of the corresponding column. Last column specifies origin of the transformed coordinate system. Last row of the transform is always [0, 0, 0, 1].
- Translation and rotation sliders:
  - Translation: Apply LR, PA, and IS translational components of the transformation matrix in RAS space (in mm). Min and Max control the lower and upper bounds of the sliders.
  - Rotation: Apply LR, PA, and IS rotation angles (degrees) in the RAS space. Rotations are concatenated.
  - Note: Linear transform edit sliders only show relative translation and rotation because a transformation can be achieved using many different series of transforms. To make this clear to users, only one transform slider can be non-zero at a time (all previously modified sliders are reset to 0 when a slider is moved). The only exception is translation sliders in "translate first" mode (i.e., when translation in global/local coordinate system button is not depressed): in this case there is a only one way how a specific translation can be achieved, therefore transform sliders are not reset to 0. An rotating dial widget would be a more appropriate visual representation of the behavior than sliders, but slider is chosen because it is a standard widget and users are already familiar with it.
- Translation reference frame: Determines what coordinate system the translation (specified by translation sliders) is specified in - in the parent coordinate system or in the rotated coordinate system.
- Identity: Resets transformation matrix to identity matrix.
- Invert: Inverts the transformation matrix.
- Split: Split a composite transform so that each of its component is stored in a separate transform node.
- Copy: Copy the homogeneous transformation matrix values to the clipboard. Values are separated by spaces, each line of the transform is in a separate line.
- Paste: Paste transformation matrix from clipboard. Values can be separated by spaces, commas, or semicolons, which allows copy/pasting numpy array from Python console or matrix from Matlab.
- Scattered transform: for a transform that is computed from displacements at the points of a model, the source model, the displacement array, the interpolation, the displacement scale, the fitting parameters (in the Fitting group), and how closely the fit follows the displacements. See [Compute transform](transforms.md#compute-transform).
- Center of transformation: The center of transformation of the selected transform. This is the position that the sliders will rotate around, and where the interaction handles are positioned. The center of transformation can be edited/displayed in world or local coordinates. Clicking the 'Reset' button will reset the center of transformation to the local coordinates [0,0,0].

![Widget for editing linear transforms.](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_transforms_edit.png)

![Widget for editing/displaying the center of transformation.](https://github.com/Slicer/Slicer/releases/download/docs-resources/center_of_transformation.png)

### Display

This section allows visualization of how much and what direction of displacements specified by the transform, and controls the interaction handles that can be used for moving the transform in the views.

#### Displacement field

This section shows the displacement vectors of the transform in slice and 3D views. The same controls are used for displaying vector volumes (see [Volumes module](volumes.md)) and vector arrays of models (see [Models module](models.md)).

![Transforms module section for controlling transform display options.](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_transforms_display_2024_03_26.png)

- **Visibility**: show the displacement field in the views.
- **Visibility in 3D view**: show the displacement field in 3D views.
- **Visibility in slice view**: show the displacement field in slice views.
- **Mode**: choose how the displacement field is drawn: **Glyphs**, **Grid**, or **Contour**. See [Visualization modes](transforms.md#visualization-modes) below.
- **Sample at**: node that the displacement field is sampled at: glyphs are placed at the samples, and the grid and contours are built over it. A point list, a curve, a plane, a markups ROI, a slice view, a model, a segmentation, or a volume. Since a transform is defined everywhere, nothing is drawn in 3D views until a node is selected. Slice views show the displacement field wherever the slice plane is.
- **Sampling**: which part of the node is sampled: its **Points**, its **Surface**, or its **Inside** (only the parts that the node has are offered), and how: **Regular** (every point, points about **Spacing** apart on the surface of a model or segmentation, or a lattice or steps along a curve **Spacing** apart; the same spacing is the resolution of the grid and contours, and in grid mode it controls how smoothly the grid lines follow the deformation, not how far apart they are) or **Random** (**Number of points** samples spread evenly at random). The grid and contours are always built on a lattice, over the inside of the node or in its plane.
- **Mask**: only show the displacement field inside (or outside) of this node, in every mode and every view: a markups ROI, a closed model, a segmentation, or the extent of a volume. Streamlines are only integrated where the field is shown, and stop where they leave it. **Segments**: for a segmentation, the segments that mask the field (all of them if none is checked).
- **Scale factor**: scales the drawn displacements (the glyph length in glyph mode, the deformation of the grid in grid mode). With a value of 1 the displacements are drawn at their actual size. **Reverse** (next to the slider) draws the displacements pointing the opposite way (in grid mode: deforms the grid the opposite way, which approximately shows the inverse transform); it is stored as a negative scale factor. Not shown in contour mode.
- **Threshold**: if enabled, the displacement field is only drawn where the displacement magnitude is inside the specified range. Not available in contour mode, where the contour levels already specify which magnitudes are drawn.
- **Advanced**: settings of the selected visualization mode.
  - Glyph mode:
    - **Glyph type**: shape of the glyphs: arrow, cone, box, cylinder, line, or sphere.
  - Grid mode:
    - **Grid spacing**: distance between the grid lines.
    - **Non-warped grid visibility**: show the undeformed grid next to the deformed one, in 3D and slice views, in the color chosen next to the checkbox, whichever way the deformed grid is colored.
    - **Grid lines visibility**: draw the deformed grid lines. Turn it off to show only the fill or the undeformed grid.
    - **Fill**: fill the surface of the grid with the colors of the displacement field. With a plane or slice as **Sample at**, it is a colored map of the displacement in that plane (flat when the scale factor is 0); in slice views the slice plane is filled.
  - Contour mode:
    - **Contour levels**: displacement magnitude values (in mm), separated by spaces, where the iso-lines (slice views) and iso-surfaces (3D views) are drawn. If no levels are specified then a few levels are drawn, spread over the color range.
  - **3D display**: appearance in 3D views.
    - **Opacity**: opacity of the displayed field.
    - **Grid line diameter**: diameter of the grid lines (grid mode). 0 draws thin lines.
    - **Glyph diameter**: thickness of the glyphs. If **absolute** is pressed then all glyphs have the same thickness, specified in mm; otherwise the thickness is a percentage of the glyph length, so longer glyphs are thicker.
    - **Shaft diameter**, **Tip length**, **Resolution**: geometry of the glyphs: shaft diameter of arrows (as percentage of the tip diameter), tip length (as percentage of the arrow length), and number of sides. Only the settings that apply to the selected glyph type are shown.
  - **Slice display**: appearance in slice views.
    - **Opacity**: opacity of everything drawn in slice views (glyphs, grid lines and fill, isolines, streamlines).
    - **Line width**: line width of glyphs in slice views.
    - **Tip length**, **Resolution**: geometry of the flat glyphs drawn in slice views.
- **Coloring**:
  - **Color by**: draw the displacement field with a single color (**Solid color**, specified by **Color**) or colored by the displacement **Magnitude**. The box next to "Active Scalar" chooses between the magnitude and one component of the displacement (0, 1, 2 for the R, A, S components).
  - Color map: specifies the color of each displacement magnitude value. See [Coloring](transforms.md#coloring) below.
  - **Color legend**: show a color legend in the views that explains the meaning of the displacement field colors. Only available when the field is colored by magnitude.

The same transform can be shown in multiple ways (for example, both contours and grid or glyph representations) by loading the same transform twice and choosing a different visualization mode for each.

Display settings of transforms in scenes saved by earlier Slicer versions are preserved when the scene is loaded.

#### Interaction

![Display options widget for controlling interaction handle visibility.](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_transforms_display_interaction.png)

The interaction section controls the visibility of interaction handles for manual positioning of transforms. Individual axes can be enabled/disabled separately for 3D and Slice views by clicking the "More options..." button.

In the slice view's "More options" panel, the "Enable view plane translation anywhere" option allows adjusting translation by left-clicking and dragging anywhere in the slice view, instead of only by grabbing the small handle at the center of the transform. While this option is enabled, that handle is drawn as a crosshair (with an empty center) instead of a filled circle. This option is disabled by default, and only takes effect if translation is enabled for the slice view. The "View plane translation scale" slider (default 0.2) controls how much the transform moves relative to the mouse cursor movement while dragging anywhere; it does not affect dragging handles.

- Size: Size of the interaction handles, relative to the application window size (or as an absolute size in mm, if `absolute` is enabled).
- Opacity: Overall opacity of the interaction handles, in addition to the fade effect that is already applied when a handle is viewed edge-on.

#### Visualization modes

1. Glyph mode

Each glyph shows the displacement vector at a sampling point. By default, the glyph length is the displacement length multiplied by the scale factor.

Slice view: glyphs are drawn as flat shapes in the slice plane and show the component of the displacement vector that lies in the slice plane (projection of the vector to the slice plane). Glyphs are colored by the magnitude of the full 3D displacement vector, so that the colors are the same as in 3D views.
- Arrow: the arrow shows the displacement vector at the arrow starting point, projected to the slice plane
- Cone: the cone shows the displacement vector at the cone centerpoint, projected to the slice plane
- Sphere: the circle diameter shows the displacement vector magnitude at the circle centerpoint

| Glyph - arrow (slice view)   | Glyph - cone (slice view)   | Glyph - sphere (slice view)      |
|------------------------------|-----------------------------|----------------------------------|
| ![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_transforms_glyph_arrow_2d.png)  | ![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_transforms_glyph_cone_2d.png)  | ![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_transforms_glyph_sphere_2d.png)  |

3D view:
- Arrow: the arrow shows the displacement vector at the arrow starting point
- Cone: the cone shows the displacement vector at the cone centerpoint
- Sphere: the sphere diameter shows the displacement vector magnitude at the sphere centerpoint

| Glyph - arrow (3D view)   | Glyph - cone (3D view)   | Glyph - sphere (3D view)      |
|------------------------------|-----------------------------|----------------------------------|
| ![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_transforms_glyph_arrow_3d_slice.png)  | ![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_transforms_glyph_cone_3d_roi.png)  | ![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_transforms_glyph_sphere_3d_volume.png)  |

2. Grid mode

- Slice view: Grid visualization (2D): shows a regular grid, deformed by the displacement vector projected to the slice
- 3D view: shows a regular grid, deformed by the displacement vector

| Grid (slice view)   | Grid - slice plane (3D view)   | Grid - volume (3D view)      |
|------------------------------|-----------------------------|----------------------------------|
| ![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_transforms_grid_2d.png)  | ![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_transforms_grid_3d_slice.png)  | ![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_transforms_grid_3d_volume.png)  |

3. Contour mode

- Slice view: iso-lines corresponding to selected displacement magnitude values
- 3D view: iso-surfaces corresponding to selected displacement magnitude values

| Contour (slice view)         | Contour (3D view)           |
|------------------------------|-----------------------------|
| ![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_transforms_contour_2d.png)  | ![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_transforms_contour_3d_volume.png) |

Streamline mode is available for vector volumes and models, but not for transforms.

#### Coloring

Open Transforms module / Display section / Displacement field / Coloring section and set "Color by" to "Magnitude". If you click on a small circle in the color map then above the color bar you can see the small color swatch. On its left side is the points index (an integer that tells which point is being edited and that can be used to jump to the previous/next point), and on its right side is the mm value corresponding to that color.

The default colormap is:
- 1mm (or below) = gray
- 2mm = green
- 5mm = yellow
- 10mm (or above) = red

You can drag-and-drop any of the small circles or modify the mm value in the editbox. You can also add more color values by clicking on the color bar. Then, you can assign a color and/or adjust the mm value. If you click on a circle and press the DEL key then the color value is deleted.

If you need to know accurate displacement values at specific positions then switch to contour mode and in the "Contour levels" field (in the Advanced section) enter all the mm values that you are interested in. For example, if you enter only a single value "3" you will see a curve going through the points where the displacement is exactly 3 mm; on one side of the curve the displacements are smaller, on the other side the displacements are larger.

### Apply transform

Controls what nodes the currently selected Transform is applied to.

- Transformable: List the nodes in the scene that *do not* directly use the selected transform node.
- Transformed: List the nodes in the scene that use the selected transform node.
- Right arrow: Apply the selected transform node to the selected nodes in Transformable list.
- Left arrow: Remove the selected transform node from the selected nodes in the Transformed list.
- Harden transform: Harden selected transform on the nodes selected in the Transformed list.

### Convert

- Operation: Export the selected transform to a node, or import it from a node.
- Export: the transform is sampled on the grid of the "Reference volume" and saved to the "Output node". If no output node is selected then a new one is created. Output type:
  - Grid transform: a grid transform is constructed. This can be used for creating an inverted displacement field that any ITK application can read. When the grid transform is written to file, both the image grid and displacement values are saved in LPS coordinate system.
  - Magnitude volume: magnitude of displacement is saved as a scalar volume.
  - Vector volume: displacement vectors are saved as voxel values (in RAS coordinate system). When the vector volume is written to file, the image grid is saved in LPS coordinate system, but displacement values are still kept in RAS coordinate system.
- Import: the selected transform is set from the "Input node". Input type:
  - Vector volume: the displacement vectors of the voxels (in RAS) become a grid transform, as the transform to parent; exporting it again to a vector volume on the same grid gives the same volume. A volume under a linear transform is imported where it is shown: the grid is moved, and the displacement vectors are rotated, with it.
  - Model: a smooth b-spline transform is fitted to the displacement vectors stored in a point data array ("Displacement array") of the model. It follows later changes of the mesh, and its parameters can be changed in the Edit section. See [Compute transform](transforms.md#compute-transform).

## Related modules

- [Registration modules](../registration): transforms can be computed automatically using semi-automatic or automatic registration modules

## Information for developers

See examples and other developer information in [Developer guide](../../developer_guide/modules/transforms) and [Script repository](../../developer_guide/script_repository.md#transforms).

## Contributors

- Alex Yarmarkovich (Isomics, SPL)
- Jean-Christophe Fillion-Robin (Kitware)
- Julien Finet (Kitware)
- Andras Lasso (PerkLab, Queen's)
- Franklin King (PerkLab, Queen's)
- Kyle Sunderland (PerkLab, Queen's)

## Acknowledgements

This work is funded in part by the National Institutes of Health through the NIH Roadmap for Medical Research, Grant U54 EB005149.

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_isomics.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_kitware.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_namic.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_nac.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_perklab.png)
