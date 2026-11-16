# Transforms

## Overview

This module is used for creating, editing, and visualization of spatial transformations. Transformations are stored in transform nodes and define position, orientation, and warping in the world coordinate system or relative to other nodes, such as volumes, models, markups, or other transform nodes.

See a summary of main features demonstrated in [this video](https://screencast.com/t/Z6dQVjK3m).

Supported transform types:
- linear transform: rigid, scaling, shearing, affine, etc. specified by a 4x4 homogeneous transformation matrix
- b-spline transform: displacement field specified at regular grid points, with b-spline interpolation
- grid transform: dense displacement field, with trilinear interpolation
- thin-plate splines: displacements specified at arbitrarily placed points, with thin-plate spline interpolation
- model-based transform: displacements stored as a point data array of a model (for example the displacement of a solid in a simulation), interpolated by a smooth b-spline fit or exactly in the cells of the mesh. See [Compute transform](transforms.md#compute-transform).
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
  - translation and rotation of a linear transform can be interactively edited in 3D by enabling "Visible in 3D view" in Transform's module Display / Interaction section. See [Interaction section](transforms.md#interaction).
- Edit warping transform: to specify/edit a warping transform that translates a set of points to specified positions, you can use [semi-automatic registration methods](../registration.md#semi-automatic-registration)

### Compute transform

Transforms are usually computed using [spatial registration tools](../registration.md).

A transform can also be computed from displacements stored in a model, for example the displacement of a solid computed by a simulation, so that the displacement can be applied to other models or images, or displayed outside the solid. Such a model-based transform is an ordinary transform node whose transform is computed from a 3-component point data array of the mesh of a model (its "displacement model"):

- `vtkScatteredBSplineTransform` (vtkAddon): a smooth cubic b-spline is fitted to the displacements at the mesh points (multilevel b-spline approximation). It is defined everywhere: between the points, and beyond them, where it follows the overall linear trend of the displacements and then falls to zero at the border falloff distance from the points (see below).
- `vtkMeshDisplacementTransform`: the displacements are used as they are, nothing is fitted. The points of the mesh are moved by their own vectors (so the model itself, placed under the transform, is warped exactly as by its vectors), and any other position by the vector interpolated in the cell of the mesh that contains it. Positions outside the cells are not moved (unless a border falloff distance is set, see below), so this is meant for volumetric meshes, or for warping the model itself. It is much slower to evaluate than the b-spline fit, because each position near the mesh needs a search in the mesh: for example, resampling a 512x512x321 CT under a transform from a heart valve simulation mesh took about 15 minutes with it and a few seconds with the b-spline fit, whose displacements differed from the exact ones by 0.06 mm on average (0.34 mm at most) for displacements up to 12 mm. Use the b-spline fit for images, and this one where the displacements must be followed exactly.

The transform follows the model: it is computed again when the mesh changes (also when another mesh is set, for example along a sequence), and everything transformed by it is updated. The transform holds a randomly generated key, which the transform node uses to reference the model. The key is saved and copied with the transform, so the transform is computed again from the same model when the scene is loaded, when it is restored from a sequence or a scene view, and when it is hardened or split. A transform node can have several model-based transforms, for example after a model-based transform is hardened on another one; each follows its own model.

To create one, select (or create) a transform in the Transforms module, and in the "Convert" section choose "Import", "Model", the model and the displacement array. The model, the array and the parameters can then be changed in the Edit section. From the Python console:

```python
modelNode = getNode("solid")
transformNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLTransformNode")
slicer.modules.transforms.logic().ImportTransformFromModel(transformNode, modelNode, "Displacement")
transform = transformNode.GetTransformToParent()  # a vtkScatteredBSplineTransform
print("Displacement model:", transformNode.GetDisplacementModelNode(transform).GetName())
transform.Update()
print("Fit error [mm]:", transform.GetResidual())
# Warp another model, or an image, by the displacements
getNode("otherModel").SetAndObserveTransformNodeID(transformNode.GetID())
```

`ImportTransformFromModel(..., keepLinked=False)` instead sets a copy of the fit, an ordinary b-spline transform that does not follow later changes of the model and that other applications can read too.

A model-based transform can also be set up directly: `transformNode.SetAndObserveModelBasedTransformToParent(transform, modelNode)`, where `transform` is a `vtkScatteredBSplineTransform` or a `vtkMeshDisplacementTransform`. Like `SetAndObserveTransformToParent`, it replaces the whole transform of the node, including any linear or composite transform that it had. `SetAndObserveDisplacementModelNode(transform, modelNode)` changes the model of a model-based transform of the node, for example of a component of a composite transform: to combine a model-based transform with other transforms, set a `vtkGeneralTransform` that has it as a component with `SetAndObserveTransformToParent`, then set its model with `SetAndObserveDisplacementModelNode`.

Parameters (in the Edit section, "Displacement scale" and the "Fitting" group; the values are set from the points when the transform is created):

| Parameter | GUI | Meaning | Default |
|---|---|---|---|
| transform class | Method | `vtkScatteredBSplineTransform` ("Smooth fit") or `vtkMeshDisplacementTransform` ("Exact"), see above. The parameters below only apply to the smooth fit, except `DisplacementScale` and the border falloff distance, which apply to both. | Smooth fit |
| `DisplacementScale` | Displacement scale | multiplies the displacements: 0 is the identity, 2 twice the fitted displacements; for exaggerating or animating a deformation. **Reverse** (next to the slider) makes it negative, moving points the opposite way. The fit is not affected. | 1 |
| `BorderFalloffDistance`, `AutomaticBorderFalloffDistance` | Border falloff distance, Automatic | distance from the mesh at which the displacement has faded to zero, in mm; it should be larger than about 1.5 times the largest displacement. Smooth fit: the fit is made zero at this distance from the points, so the transform follows the shape of the mesh, not its bounding box; 0 fits over the whole extrapolation region instead. Exact: a position outside the cells moves by the displacement of the closest point of the mesh, fading smoothly to zero at this distance; 0 does not move positions outside the cells, so the displacement jumps to zero at the border of the mesh, which can make the inverse (used when images are resampled) fail there. **Automatic** uses twice the largest displacement (times the displacement scale), which follows the model when it changes; the distance in use is shown. | automatic |
| `GridBoundsMargin` | Extrapolation distance | how far beyond the points the transform is fitted, in mm; with a border falloff the fitted region extends at least 1.25 times the falloff distance | a tenth of the diagonal of the bounding box of the points |
| `ControlPointSpacing` | Coarsest spacing | b-spline control point spacing of the coarsest level, along each axis, in mm; each further level halves it | 0 = a tenth of the size of the fitted region along that axis |
| `MinimumControlPointSpacing` | Finest spacing | refinement stops before the spacing gets smaller than this, in mm | a hundredth of the size of the fitted region |
| `MaximumNumberOfLevels` | Maximum levels | largest number of refinement levels (1-16); each level needs about 8 times as much memory | 8 |
| `Tolerance` | Tolerance | refinement stops when the largest error at the points is below this, in mm | a thousandth of the largest displacement |
| `UseLinearApproximation` | Linear approximation | start from the best-fitting linear transform, so that the transform follows the linear trend of the displacements away from the points instead of flattening out | on |
| `BorderMode` | Border mode | beyond the fitted region: keep the displacement at the border (`Edge`), no displacement (`Zero`), or fade to zero at the border (`ZeroAtBorder`) | `ZeroAtBorder` |

The Edit section also shows the fit: the largest difference between the transform and the displacements at the points (`GetResidual()`), and the number of refinement levels it used (`GetNumberOfLevels()`). To use the transform in other applications, export it to a grid transform in the "Convert" section.

### Save transform

Transforms are saved into [ITK](https://itk.org/) transform file format. ITK always saves transform "from parent", as this is the direction that is necessary for transforming images.

If a transform node in Slicer has a transformation with "to parent" direction (e.g., it was computed like that or a "from parent" transform got inverted) then:
- linear transforms: the transform is automatically converted to "from parent" direction.
- warping transforms: the transform is saved as special inverse transform class that most ITK-based applications cannot interpret. If compatibility with other applications is needed, the transform can be converted to a displacement field before saving.

Model-based transforms (computed from the displacements of a model, see above) are saved as special transform classes (`MeshDisplacementTransform`, `ScatteredBSplineTransform`, and their inverse) that hold only the parameters of the transform: the transform is computed from its displacement model when it is loaded, so the file can only be used with the scene that has the model. Other applications cannot interpret it: the transform can be converted to a displacement field for them. The file format is described in the [Developer guide](../../developer_guide/modules/transforms.md#model-based-transform-files).

A quick way to export a linear transform to another software or to text files is to copy the homogeneous transformation matrix values to the clipboard by clicking "Copy" button in Edit section in Transforms module.

### Load transform

Drag-and-drop the transform file to the application window and make sure "Transform" is selected in Description column.

MetaImage (mha), NIFTI (nii) vector volumes can be loaded as displacement field (grid) transform. The volume and its vectors are expected to be stored in LPS coordinate system (during reading they are converted to RAS to match coordinate system conventions in Slicer).

Sequences of displacement field (grid) transforms can be loaded from and saved to NRRD sequence (.seq.nrrd) and NIfTI (.nii, .nii.gz) files. See [supported transform file formats](../data_loading_and_saving.md#transforms) for details.

A quick way to import a linear transform from another software or from text files is to copy the homogeneous transformation matrix values to the clipboard, and click "Paste" button in Edit section in Transforms module.

### Visualize transform

The displacement field of a transform can be visualized in both 2D and 3D views, as glyphs representing the displacement vectors as arrows, cones, or spheres; regular grids that are deformed by the transform; or contours that represent lines or surfaces where the displacement magnitude has a specific value, in the [Vector Field Rendering module](vectorfieldrendering.md). The **Show displacement field in Vector Field Rendering module** button in the [Display section](transforms.md#display) opens it with the transform selected. Linear transforms can also be interacted with using an interaction handle widget. See [Interaction section](transforms.md#interaction)

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
- Model-based transform: for a transform that is computed from displacements at the points of a model, the source model, the displacement array, the method, the displacement scale, the fitting parameters (in the Fitting group), and how closely the fit follows the displacements. See [Compute transform](transforms.md#compute-transform).
- Center of transformation: The center of transformation of the selected transform. This is the position that the sliders will rotate around, and where the interaction handles are positioned. The center of transformation can be edited/displayed in world or local coordinates. Clicking the 'Reset' button will reset the center of transformation to the local coordinates [0,0,0].

![Widget for editing linear transforms.](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_transforms_edit.png)

![Widget for editing/displaying the center of transformation.](https://github.com/Slicer/Slicer/releases/download/docs-resources/center_of_transformation.png)

### Display

This section controls the interaction handles of the transform. **Show displacement field in Vector Field Rendering module** shows how much and in what direction the transform displaces points, in the [Vector Field Rendering module](vectorfieldrendering.md), with the transform selected there.

#### Interaction

![Display options widget for controlling interaction handle visibility.](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_transforms_display_interaction.png)

The interaction section controls the visibility of interaction handles for manual positioning of transforms. Individual axes can be enabled/disabled separately for 3D and Slice views by clicking the "More options" button.

In the slice view's "More options" panel, the "Enable view plane translation anywhere" option allows adjusting translation by left-clicking and dragging anywhere in the slice view, instead of only by grabbing the small handle at the center of the transform. While this option is enabled, that handle is drawn as a crosshair (with an empty center) instead of a filled circle. This option is disabled by default, and only takes effect if translation is enabled for the slice view. The "View plane translation sensitivity" slider (default 0.2) controls how much the transform moves relative to the mouse cursor movement while dragging anywhere; it does not affect dragging handles.

- Size: Size of the interaction handles, relative to the application window size (or as an absolute size in mm, if `absolute` is enabled).
- Opacity: Overall opacity of the interaction handles, in addition to the fade effect that is already applied when a handle is viewed edge-on.

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
