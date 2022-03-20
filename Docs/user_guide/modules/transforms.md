# Transforms

## Overview

This module is used for creating, editing, and visualization of spatial transformations. Transformations are stored in transform nodes and define position, orientation, and warping in the world coordinate system or relative to other nodes, such as volumes, models, markups, or other transform nodes.

See a summary of main features demonstrated in [this video](https://screencast.com/t/Z6dQVjK3m).

Supported transform types:
- linear transform: rigid, scaling, shearing, affine, etc. specified by a 4x4 homogeneous transformation matrix
- b-spline transform: displacement field specified at regular grid points, with b-spline interpolation
- grid transform: dense displacement field, with trilinear interpolation
- thin-plate splines: displacements specified at arbitrarily placed points, with thin-plate spline interpolation
- composite transforms: any combinations of the transforms above, in any order, any of them optionally inverted

## Use cases

### Create a transform

Transform node can be created in multiple ways:
- Method A: In Data module's Subject hierarchy tab, right-click on the "Transform" column and choose "Create new transform". This always creates a general "Transform".
- Method B: In Data module's Transform hierarchy tab, right-click on an item and choose "Insert transform". This always creates a "Linear transform". Advantage of this method is that it is easy to build and overview hierarchy of transforms.
- Method C: In Transforms module click on "Active transform" node selector and choose one of the "Create new..." options.

How to choose transform type: Create "Linear transform" if you only work with linear transforms, because certain modules only allow you to select this node type as input. In other cases, it is recommended to  create the general "Transform" node. Multiple transform node types exist because earlier Slicer could only store a simple transformation in a node. Now a transform node can contain any transformation type (linear, grid, bspline, thin-plate spline, or even composite transformation - an arbitrary sequence of any transformations), therefore transform node types only differ in their name. In some modules, node selectors only accept a certain transform node type, therefore it may be necessary to create that expected transform type, but in the long term it is expected that all modules will accept the general "Transform" node type.

### Apply transform to a node

"Applying a transform" means setting parent transform to a node to translate, rotate, and/or warp it. If the parent transform is changed then the position or shape of the node is automatically updated. If the transform is removed then original (non-transformed) state of the node is restored.

A transform can be applied to a node in multiple ways:
- Method A: In Data module's Subject hierarchy tab, right-click on the "Transform" column and choose a transform (or "Create new transform"). The transform can be interactively edited in 3D views by right-click on "Transform" column and choosing "Interaction in 3D view". See this **[short demonstration video](https://youtu.be/bbikx7Edv4g)**.
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
  - translation and rotation of a linear transform can be interactively edited in 3D by enabling "Visible in 3D view" in Transform's module Display / Interaction section. See this **[short demonstration video](https://youtu.be/bbikx7Edv4g)**.
- Edit warping transform: to specify/edit a warping transform that translates a set of points to specified positions, you can use [semi-automatic registration methods](../registration.md#semi-automatic-registration)

### Compute transform

Transforms are usually computed using [spatial registration tools](../registration.md).

### Save transform

Traansforms are saved into [ITK](https://itk.org/) transform file format. ITK always saves transform "from parent", as this is the direction that is necessary for transforming images.

If a transform node in Slicer has a transformation with "to parent" direction (e.g., it was computed like that or a "from parent" transform got inverted) then:
- linear transforms: the transform is automatically converted to "from parent" direction.
- warping transforms: the transform is saved as special inverse transform class that most ITK-based applications cannot interpret. If compatibility with other applications is needed, the transform can be converted to a displacement field before saving.

A quick way to export a linear transform to another software or to text files is to copy the homogeneous transformation matrix values to the clipboard by clicking "Copy" button in Edit section in Transforms module.

### Load transform

Drag-and-drop the transform file to the application window and make sure "Transform" is selected in Description column.

MetaImage (mha), NIFTI (nii) vector volumes can be loaded as displacement field (grid) transform. The volume and its vectors are expected to be stored in LPS coordinate system (during reading they are converted to RAS to match coordinate system conventions in Slicer).

A quick way to import a linear transform from another software or from text files is to copy the homogeneous transformation matrix values to the clipboard, and click "Paste" button in Edit section in Transforms module.

### Visualize transform

Transforms can be visualized in both 2D and 3D views, as glyphs representing the displacement vectors as arrows, cones, or spheres; regular grids that are deformed by the transform; or contours that represent lines or surfaces where the displacement magnitude has a specific value. See documentation of [Display section](transforms.md#display] for details.

## Panels and their use

Active Transform: Select the transform node to display, control and edit.

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
- Copy: copy the homogeneous transformation matrix values to the clipboard. Values are separated by spaces, each line of the transform is in a separate line.
- Paste: paste transformation matrix from clipboard. Values can be separated by spaces, commas, or semicolons, which allows copy/pasting numpy array from Python console or matrix from Matlab.

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_transforms_edit.png)

### Display

This section allows visualization of how much and what direction of displacements specified by the transform.

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_transforms_display.png)

#### Visualization modes

1. Glyph mode

Slice view:
- arrow: the arrow shows the displacement vector at the arrow starting point, projected to the slice plane
- cone: the cone shows the displacement vector at the cone centerpoint, projected to the slice plane
- sphere: the circle diameter shows the displacement vector magnitude at the circle centerpoint

| Glyph - arrow (slice view)   | Glyph - cone (slice view)   | Glyph - sphere (slice view)      |
|------------------------------|-----------------------------|----------------------------------|
| ![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_transforms_glyph_arrow_2d.png)  | ![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_transforms_glyph_cone_2d.png)  | ![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_transforms_glyph_sphere_2d.png)  |

3D view:
- arrow: the arrow shows the displacement vector at the arrow starting point
- cone: the cone shows the displacement vector at the cone centerpoint
- sphere: the sphere diameter shows the displacement vector magnitude at the circle centerpoint

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

#### Coloring

Open Transforms module / Display section / Colors section. If you click on a small circle then above the color bar you can see the small color swatch. On its left side is the points index (an integer that tells which point is being edited and that can be used to jump to the previous/next point), and on its right side is the mm value corresponding to that color.

The default colormap is:
- 1mm (or below) = gray
- 2mm = green
- 5mm = yellow
- 10mm (or above) = red

You can drag-and-drop any of the small circles or modify the mm value in the editbox. You can also add more color values by clicking on the color bar. Then, you can assign a color and/or adjust the mm value. If you click on a circle and press the DEL key then the color value is deleted.

If you need to know accurate displacement values at specific positions then switch to contour mode and in the “Levels” list enter all the mm values that you are interested in. For example, if you enter only a single value “3” in the Levels field you will see a curve going through the points where the displacement is exactly 3 mm; on one side of the curve the displacements are smaller, on the other side the displacements are larger.

You can show both contours and grid or glyph representations by loading the same transform twice and choosing a different representation for each.

### Apply transform

Controls what nodes the currently selected "Active transform" is applied to.

- Transformable: List the nodes in the scene that *do not* directly use the active transform node.
- Transformed: List the nodes in the scene that use the active transform node.
- Right arrow: Apply the active transform node to the selected nodes in Transformable list.
- Left arrow: Remove the active transform node from the selected nodes in the Transformed list.
- Harden transform: Harden active transform on the nodes selected in the Transformed list.

### Convert

This section can sample the active transform on a grid (specified by the selected "Reference volume") and save it to a node. Depending on the type of selected "Output displacement field" node, slightly different information is exported:
- Scalar volume node : magnitude of displacement is saved as a scalar volume
- Vector volume node: displacement vectors are saved as voxel values (in RAS coordinate system). When the vector volume is written to file, the image grid is saved in LPS coordinate system, but displacement values are still kept in RAS coordinate system.
- Transform node: a grid transform is constructed. This can be used for creating an inverted displacement field that any ITK application can read. When the grid transform is written to file, both the image grid and displacement values are saved in LPS coordinate system.

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

## Acknowledgements

This work is funded in part by the National Institutes of Health through the NIH Roadmap for Medical Research, Grant U54 EB005149.

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_isomics.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_kitware.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_namic.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_nac.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_perklab.png)
