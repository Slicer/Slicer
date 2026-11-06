# Advanced Topics

## Memory Management


### Pointers to VTK object

If you are not familiar with VTK's memory management read this [general introduction](https://aosabook.org/en/vtk.html) and this [page describing usage of smart pointers](https://www.vtk.org/Wiki/VTK/Tutorials/SmartPointers). See Slicer-specific use cases and recommendations below.

Calling the plain `New()` method of VTK objects and storing the returned pointer in a plain pointer should be avoided, as this very often causes memory leaks.

Bad, should be avoided:

```cpp
vtkMRMLScalarVolumeNode* vol = vtkMRMLScalarVolumeNode::New();
// ... do something, such as vol->GetImageData(), someObject->SetVolume(vol)...
vol->Delete();
vol=NULL;
```

Recommended:

```cpp
vtkNew<MRMLScalarVolumeNode> vol;
//... do something, such as vol->GetImageData(), someObject->SetVolume(vol.GetPointer())...
```

Also good:

```cpp
vtkSmartPointer<MRMLScalarVolumeNode> vol=vtkSmartPointer<MRMLScalarVolumeNode>::New();
///... do something, such as vol->GetImageData(), someObject->SetVolume(vol)...
```

`vtkNew` is preferred in general for new object creation, as it has a simpler syntax and used almost exclusively in the Slicer core source code. A slight inconvenience is that when we need use the `GetPointer()` method to get the actual pointer value.

A `vtkSmartPointer` pointer can be created without actually creating an object, so `vtkSmartPointer` should be used when:

* we don't know the exact object type that we want to create at the time of the pointer creation (e.g., we create a `vtkSmartPointer<vtkMRMLVolumeNode>` and later set it to point to a `vtkMRMLScalarVolumeNode` or `vtkMRMLVectorVolumeNode`), or
* we need to take the ownership of an already created object (using `vtkSmartPointer::Take(...)`; see examples below)

### Factory methods

Similarly to VTK, Slicer contains some "factory" methods:

- `vtkMRMLScene::CreateNodeByClass()`
- `vtkMRMLScene::GetNodesByClass()`
- ...

Factory methods return a pointer to a VTK object (with a reference count of 1) that the caller "owns", so the caller must take care of releasing the object to avoid [memory leak](https://en.wikipedia.org/wiki/Memory_leak).

In C++, it is recommended to make a smart pointer take the ownership of the returned raw pointer. For example:

```cpp
// GetNodesByClass is a factory method, therefore a smart pointer is used to take the ownership of the returned object
vtkSmartPointer<vtkCollection> nodes = vtkSmartPointer<vtkCollection>::Take(scene->GetNodesByClass("vtkMRMLModelNode"));
```

In Python, the returned Python object maintains a reference to the underlying VTK object, therefore an extra reference is no longer needed and it is recommended to be immediately removed using the `UnRegister` method:

```python
nodes = scene.GetNodesByClass("vtkMRMLModelNode")
nodes.UnRegister(None) # GetNodesByClass method is NOT marked with VTK_NEWINSTANCE, manual unregistration is needed
```

Here are the different naming conventions for such "factory" methods:
* `GetXXX`: Return an existing object, reference count is not changed, the caller is not responsible for the object.
* `NewXXX`: Solely instantiate an object (typically a factory method). The caller is responsible to decrement the reference count.
* `CreateXXX`: Instantiate and configure an object. The caller is responsible to decrement the reference count. If `XXX` is a MRML node, it is NOT added into the scene.
* `CreateAndAddXXX`: Instantiate, configure and add into the scene a MRML node. The caller is not responsible to decrement the reference count.

#### VTK_NEWINSTANCE wrapper hint

If a factory method is marked with the `VTK_NEWINSTANCE` hint then the ownership is transferred to Python where garbage collection takes care of deleting the object when it is no longer needed.
Calling `object.UnRegister(None)` is prohibited, as it would prematurely delete the object and may crash the application.
In C++, the `VTK_NEWINSTANCE` hint has no effect, the caller of the factory method must still take the ownership of the returned object the same way as without the hint.

```python
box = roiNode.CreateROIBoxPolyDataWorld()
# no need to call UnRegister, as CreateROIBoxPolyDataWorld method is marked with VTK_NEWINSTANCE
```

#### Loadable modules (C++)

If storing in a new variable:

```cpp
vtkSmartPointer<vtkCollection> nodes = vtkSmartPointer<vtkCollection>::Take(mrmlScene->GetNodesByClass("vtkMRMLLinearTransformNode"));
```

If the variable is created already:

```cpp
vtkSmartPointer<vtkCollection> nodes;
nodes.TakeReference(mrmlScene->GetNodesByClass("vtkMRMLLinearTransformNode"));
```

Unsafe, legacy method without using smart pointers (not recommended, because the `Delete()` method may be forgotten or skipped due to an early return from the function):

```cpp
vtkCollection* nodes = mrmlScene->GetNodesByClass("vtkMRMLLinearTransformNode");
// ...
nodes->Delete();
```

#### Python scripts and scripted modules

Factory methods return an object that the caller owns (and thus the caller has to delete, with reference count >0) and Python adds an additional reference to this object when stored in a Python variable, resulting in a reference count of >1. To make sure that the object is deleted when the Python variable is deleted, we have to remove the additional reference that the factory method added. There is currently no automatic mechanism to remove that additional reference, so it has to be done manually by calling `UnRegister` (the reference count shouldn't be explicitly set to any specific value, it is only allowed to increment/decrement it using `Register`/`UnRegister`):

```py
nodes = slicer.mrmlScene.GetNodesByClass('vtkMRMLLinearTransformNode')
nodes.UnRegister(slicer.mrmlScene) # reference count is increased by both the factory method and the python reference; unregister to keep only the python reference
# ...
```

**To avoid forgetting the UnRegister call, it is better to avoid factory methods whenever it is possible.**

For example, instead of using the `CreateNodeByClass` factory method and call `UnRegister` manually:

```py
n = slicer.mrmlScene.CreateNodeByClass('vtkMRMLLinearTransformNode')
slicer.mrmlScene.AddNode(n)
n.UnRegister(slicer.mrmlScene)
```

this should be used:

```py
n = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLLinearTransformNode')
```

Note: MRML scene's `CreateNodeByClass` creates a node with the default settings set in the scene for that node type (using [vtkMRMLScene::AddDefaultNode](slicerapidocs:classvtkMRMLScene.html#ae302c5ed4aabb2910bc35dcc9aa2513f)).

## Working directory

Similarly to other software, the current directory associated with Slicer corresponds to the folder the application executable is started from.

Since the current working directory can be changed anytime by any module or Python package (e.g., to more conveniently write files in a specific directory) and it is not possible to enforce that the directory is restored to the original.

Slicer provides a way to reliably access the working directory at startup time, through the `startupWorkingPath` application property:

In Python:
```python
slicer.app.startupWorkingPath
```

In C++:
```cpp
qSlicerCoreApplication::startupWorkingPath()
```

## View layout definition

View layout definition describes what views (3D, slice, plot, table, etc.) should be displayed and where.
It is specified by an XML string.

A layout may contain multiple _viewports_, each viewport is a separate window, which may be displayed in the main application window or separately, for example on a second screen.

XML elements and their attributes:
- `viewports`: optional, if specified then it must be the root element. It can be used for specifying multiple viewports. It contains nested `layout` elements.
- `layout`: The layout elements describe widget containers that embed one or multiple items. Arrangement of items are specified by type attribute of the layout element. It may be root element or it may be nested in `viewports` or `item` element.
  - `type`: `vertical`, `horizontal`, `grid`, `tab`.
  - `split`: `true` or `false` (default), if `true` then the user can resize the views in it by dragging the splitter between the views. Default size can be set using `splitSize` attribute of child items. Only for `vertical` and `horizontal` layout type.
  - `name`: unique name of the layout, required if there are multiple viewports. If name is not specified then the empty string will be used as name. The empty string is a valid name, which refers to the default viewport, i.e., that is displayed in the application's main window.
  - `label`: optional, if specified then this will be used as
  - `dockable`: `true` (default) or `false` display the viewport as a dockable widget.
  - `dockPosition`: If dockable, this can make the widget be docked by default. Valid choices are `floating` (default), `top`, `bottom`, `left`, `right`, `bottom-left`, `bottom-right`, `top-left`, `top-right`.
- `item`: container for view(s) or layouts, nested in `layout` element.
  - `splitSize`: default size if split is enabled in the layout
- `view`: view widget, nested in `item` element.
  - `name`: this name is displayed in the view's title bar
  - `horizontalStretch` or `verticalStretch`: Relative size of views can be adjusted by specifying these stretch factors. The stretch factor must be an integer in the range of [0, 255].
  - `row` and `column`: row and column index. Only for `grid` layout type.
  - `class`: class of the view node, such as `vtkMRMLSliceNode`, `vtkMRMLViewNode`, `vtkMRMLTableViewNode`, `vtkMRMLPlotViewNode`.
  - `singletontag`: layout name of the view node (`1`, `2`, ... for 3D views; `Red`, `Yellow`, ... for slice views)
- `property`: contains view properties
  - `name`: property name, such as `viewlabel` (displayed in the view's title bar), `orientation` (for slice views),
  - element text: property value

### Example: Simple 4-up view layout

```xml
<layout type="vertical" split="true">
  <item>
  <view class="vtkMRMLViewNode" singletontag="1">
    <property name="viewlabel" action="default">1</property>
  </view>
  </item>
  <item>
  <view class="vtkMRMLSliceNode" singletontag="Red">
    <property name="orientation" action="default">Axial</property>
    <property name="viewlabel" action="default">R</property>
    <property name="viewcolor" action="default">#F34A33</property>
  </view>
  </item>
</layout>
```

### Example: Layout containing two viewports

```xml
<viewports>
 <!--default viewport-->
 <layout type="horizontal">
  <item>
   <view class="vtkMRMLSliceNode" singletontag="Red">
    <property name="orientation" action="default">Axial</property>
    <property name="viewlabel" action="default">R</property>
    <property name="viewcolor" action="default">#F34A33</property>
   </view>
  </item>
  <item>
   <view class="vtkMRMLViewNode" singletontag="1">
    <property name="viewlabel" action="default">1</property>
   </view>
  </item>
 </layout>
 <!--second dockable viewport-->
 <layout name="views+" type="horizontal" label="Views+" dockable="true" dockPosition="bottom">>
  <item>
   <view class="vtkMRMLSliceNode" singletontag="Red+">
    <property name="orientation" action="default">Axial</property>
    <property name="viewlabel" action="default">R+</property>
    <property name="viewcolor" action="default">#f9a99f</property>
    <property name="viewgroup" action="default">1</property>
   </view>
  </item>
  <item>
   <view class="vtkMRMLViewNode" singletontag="1+" type="secondary">
    <property name="viewlabel" action="default">1+</property>
    <property name="viewgroup" action="default">1</property>
   </view>
  </item>
 </layout>
</viewports>
```

## Event broker

The basic idea of the EventBroker is to avoid the following kind of code that shows up in GUIs and other classes that depend on events generated by other classes:

```cpp
node->AddObserver(vtkCommand::ModifiedEvent, callbackCommand)
```

### Problem

The problems with this:
- node 'owns' the observer, but the callbackCommand is opaque so it doesn't know anything about what will happen when the event is invoked
- the GUI needs to explicitly remove the observer before it is destroyed
- node is not introspectable; you cannot get a list of observers on the node (all data and methods are private)
- there's no easy way to know what side effects will happen for any Set call (either a priori or experimentally).
- there's no way to collapse events or disable them

### Solution

The EventBroker solve this by introducing a singleton that manages all observations.

The EventBroker is a singleton, available as:

```cpp
vtkEventBroker* broker = vtkEventBroker::GetInstance();
```

with the broker, you can make a call like the following:

```cpp
broker->AddObservation(node, vtkCommand::ModifiedEvent, this, callbackCommand);
```

where `node` is the subject of the observation.

:::{tip}
The broker is also available for use from Python, but for adding observers in Python, it is recommended to use the higher-level `slicer.util.VTKObservationMixin` instead.
:::

The broker does the following:
- adds `DeleteEvent` observers to both `node` and `this` so it can remove the observation automatically when either side is destroyed
- keeps an introspectable list of all observers it knows about
- has an option to keep a log of all event invocations for debugging and performance analysis
- has an option to turn off all event invocations
- has an _asynchronous_ option to queue all event invocations and invoke them later (off by default)
- has option to collapse redundant events in the queue

Future options include:
- add a timer to log the amount of time taken to process each event
- specify that some observations must be handled synchronously (e.g. for progress events that shouldn't be collapsed)

See the file [vtkEventBroker API documentation](slicerapidocs:classvtkEventBroker.html) for more information on logging options and introspection features.

### How to work with the vtkObserverManager

Discussion of Jan 27 2012:

[...] the problem wasn't that the Event Broker was used instead of going through `vtkObserverManager`, but that the `vtkObject::AddObserver()` was used with the callback command of the observer manager (that was later deleted which made the client data of the callback dirty when the event was fired).

We should eventually review the entire "observation" mechanism, there are currently 3 ways of observing vtkObjects:
- via the `vtkObserverManager` API (please everybody use that one for now)
- via the `EventBroker` directly
- via `vtkObject::AddObserver` directly

All three methods are valid but inconsistent with each other. In addition, it is still not user friendly to observe MRML nodes (using static functions as callback... how to synchronize a MRML node with a VTK widget in a displayable manager? how to observe with Python...).

Bugs related to observers:

- Volume Rendering ([Slicer issue #1572](https://github.com/Slicer/Slicer/issues/1572), [Slicer issue #1744](https://github.com/Slicer/Slicer/issues/1744))
  - Use AddObserver with vtkObserverManager callback that is deleted before an observed event is fired, crash when trying to dereference the deleted observer.
- Python ([Slicer issue #1656](https://github.com/Slicer/Slicer/issues/1656))
  - PyObject of the observer is deleted before an observed event is fired. crash in the callback
- Current design notes
  - For logics, `vtk[SetAnd]ObserveMRMLNode[Events]Macro` can only observe MRML nodes (see `vtkMRMLAbstractLogic::MRMLNodesCallback`)
  - For `QObjects` listening to `vtkObjects`, destruction must unlink observations (modules must call `setMRMLScene(0)` in destructor?)

### References

The following reference provided insight on requirements and design precedents.

* [Wikipedia definition of Observer Pattern](http://en.wikipedia.org/wiki/Observer_pattern)
* [A C++ implementation](http://sigslot.sourceforge.net/)
* [The Qt implementation](https://doc.qt.io/qt-6/signalsandslots.html)
* [Java Message Service (JMS) API](http://java.sun.com/products/jms/javadoc-102a/index.html)

## Vector field display

Vector volumes, models with vector point or cell data, and transforms are all displayed with the same vector field display node, `vtkMRMLVectorFieldDisplayNode`, which the Vector Field Rendering module provides. A node can have any number of them, each drawn on its own (for example glyphs and streamlines of the same field), after its default display node, which stays the first one. This section lists the properties of the display node, what each one does in each visualization mode, and how the data source changes that behavior.

Properties that are used in only one visualization mode use the mode name (Glyph, Streamline, Grid, Contour) and they are not discussed here, but each is described in the `vtkMRMLVectorFieldDisplayNode` documentation.

The settings are edited in the Vector Field Rendering module. Display nodes are added with `vtkSlicerVectorFieldRenderingLogic::AddVectorFieldDisplayNode()`, which also sets defaults for the kind of node, and shown or hidden with `SetVectorFieldDisplayNodeShown()`, which sets the visibility and the show mode together: a hidden vector field display node is ignored by the eye icon of its node (as volume rendering is), so showing the node does not show it again. The "Show as vector field" option of the subject hierarchy visibility menu uses the same functions. Where a property has a different label in the user interface, the label is given in parentheses. "—" means the property is not used in that mode. Lengths are in the scene's length unit (mm by default).

### Display pipeline

The field is built once for each display node, by a `vtkMRMLVectorFieldPipeline`, and shared with every view: a field source for the displayable node, a region filter that marks where the field is shown, and, for 3D views, everything up to the mappers: the points that glyphs are placed at (`vtkVectorFieldGlyph`), and the geometry of the grid, contour and streamline modes (`vtkVectorFieldGeometry`, a VTK filter with one output for the geometry, one for the streamlines as lines, which slice views share, and one for the undeformed lines of a grid). A non-linear parent transform is applied to the points there too. Each 3D view only adds a mapper and an actor, plus a second pair for the undeformed grid lines, which are drawn in `GridNonWarpedColor` whichever way the field is colored; this is the only part that has to be per view (a mapper belongs to one render window); a linear parent transform is the actor's user matrix. Each slice view samples the field in its own plane with a sampler of the same kind (or selects the mesh points near the slice), builds its own glyphs, grid lines or isolines, and draws the part of the shared streamlines that runs near the slice. Filters marked optional are only connected when the settings need them; otherwise their input goes straight through. The whole grid is built by one VTK filter, `vtkVectorFieldGrid`, in 3D and slice views alike: the lattice lines, warped by the vectors, the undeformed lines on a second output, the region clip, the fill and, in 3D views, the tubes. Glyphs are described by one VTK filter too, `vtkVectorFieldGlyph`: it selects the points that get a glyph (threshold, region test, random or spaced subset), makes the glyph shape, and sets up the `vtkGlyph3DMapper` that places the glyphs on the GPU in 3D views (`ConfigureMapper()`). Slice views use it to build the glyphs as geometry instead: flat outlines in the slice plane, scaled uniformly, along the vectors projected to the plane if `GlyphSliceProjectionEnabled` is on. The same geometry, in 3D, is what `vtkSlicerVectorFieldRenderingLogic::GetVisualizationMesh()` returns for saving the glyphs as a model or exporting them to a file; in the other modes it returns the grid, isosurfaces or streamlines that 3D views show. Where the field is sampled is worked out by `vtkMRMLVectorFieldSamplePointSource` from the sampling region node (a VTK source filter, whose sample positions and streamline seeds are computed again only when the nodes that define them change; its static functions give the same answers directly): the lattice over a region of interest, a plane, a slice or a bounding box, the points of a node, steps along a curve, or random positions in a box, on a rectangle or on a surface; it also decides which of these the settings of the display node call for (`GetSamplingKind()`) and gives the samples of the current mode and the seeds of the streamlines. The filters that bring the field from the displayable node to the views (the field source, the region filter, the transforms into the world, the samplers of the slice views) are wired by `vtkMRMLVectorFieldPipeline`; it also copies the display node settings into the glyph and geometry filters, of the 3D views and of each slice view; it also makes the fits for interpolating between the points of a mesh (`UpdateVectorFit()`, `UpdateColorFit()`) and presents the voxels of a vector volume as an image in RAS for tracing streamlines through them.

The shared filters are not in the display node, which only holds the settings: there is one `vtkMRMLVectorFieldPipeline` for each display node, which `vtkMRMLVectorFieldPipeline::GetSharedPipeline()` returns, creating it if nothing holds it. The views draw the field with pipelines of the LayerDM module (`vtkMRMLVectorField3DViewPipeline` and `vtkMRMLVectorField2DViewPipeline`), one for each view and display node, which the LayerDM displayable manager creates and removes; each holds the shared pipeline while it shows the display node. They only own what has to be per view: the mapper and actor of a 3D view, and the plane sampler, slab selection and 2D glyphs of a slice view. Their actors are in the default renderer of the view, with the other actors of the scene. Everything else that needs the field holds the shared pipeline while it uses it: the display widget for the interpolation residual, and `vtkSlicerVectorFieldRenderingLogic::GetVisualizationMesh()` for export, which works also when no view shows the node. The pipeline gives the display node the output of its field source (`GetSampledFieldDataSet()`), which the node computes its arrays and scalar range from, so they are available as long as the field has been sampled, also after the pipeline is deleted.

```{eval-rst}
.. mermaid:: diagrams/vector_field_display_pipeline.mmd
   :align: center
```

### Field source and arrays

| Property | Glyph | Streamline | Grid | Contour |
|---|---|---|---|---|
| `VisualizationMode` (Mode) | selects glyph mode | selects streamline mode (not offered for transforms) | selects grid mode | selects contour mode |
| `FieldArrayName` (Vector array) | the vector array; the glyph orientation, glyph scale and color arrays follow it unless set separately; models: nothing is drawn until a vector array of the mesh is chosen (here or as the glyph orientation array) | the vectors that are integrated | the vectors that warp the lattice | the vectors whose magnitude is contoured |
| `InterpolateBetweenPoints` (Interpolate between points) | models: fits a B-spline to the vectors at the points, so that glyphs can be placed anywhere near them; a surface mesh or point cloud needs it for anything but its own points | models: required for a surface mesh or point cloud; extends the field of a volumetric mesh beyond its cells | same as streamline | same as streamline |
| `InterpolationMaximumDistance` (Maximum distance from points) | no samples farther than this from the data (0 = automatic, 10% of the bounding box diagonal of the data) | same | same | same |

### Where the field is sampled and shown

| Property | Glyph | Streamline | Grid | Contour |
|---|---|---|---|---|
| `SamplingRegionNode` (Sample at) | where the glyphs are placed; none = the source itself | where the streamlines start; none = the points of a model (or a random subset of them), or samples in a volume; a transform needs a node | a region of interest, segmentation, model or volume (its inside), or a plane or slice: the grid is built over it; other nodes, or none: over the whole field | same as grid; not used for a volumetric mesh (without `InterpolateBetweenPoints`), which is contoured on its own cells, over the whole mesh |
| `SamplingRegionPart` (Sampling) | which part of the node is sampled: its points, its surface, or its inside; the default suits the node, see below | same as glyph | the inside, or the plane | same as grid (not used for a volumetric mesh) |
| `SamplingPattern` (Sampling) | regular: every point, or a lattice, steps along a curve or points of a surface `SamplingSpacing` apart; random: `SamplingPointsNumber` samples spread uniformly | same as glyph | — (always a lattice) | — (always a lattice) |
| `SamplingSpacing` (Spacing) | spacing of the regular pattern | spacing of the seeds; the field is traced at the resolution of the source. | lattice spacing, refined so that the grid lines fall exactly `GridSpacing` apart | lattice spacing (not used for a volumetric mesh, which is contoured on its own cells instead) |
| `SamplingPointsNumber` (Number of points) | number of random samples | number of random seeds | — | — |
| `MaskingRegionNode` (Mask) | glyphs outside it are not drawn | limits where the streamlines are integrated: they are traced through the cells of the field that it shows at all of their points, so they stop where they leave it, and seeds outside it start nothing | grid lines are cut where they leave it | isosurfaces are cut where they leave it |
| `MaskingShowInside` (Show inside, Show outside) | whether the field is shown inside or outside of the masking region | same | same | same |
| `MaskingSegmentIDs` (Segments) | the segments of a segmentation masking region; none = all of them | same | same | same |

What each kind of node gives as a sampling region (the default part is in bold):

| Node | Points | Surface | Inside |
|---|---|---|---|
| Point list, or other markups | **its control points** | — | — |
| Curve | **steps along the curve** | — | — |
| Plane, slice | — | **a lattice in its rectangle**, or random positions in it | — |
| Region of interest | — | — | **a lattice in its box**, or random positions in it |
| Model | **its mesh points** | its points, or random points spread over its area | a lattice or random positions in its bounding box, kept where they are inside the surface |
| Segmentation | — | the points of the segment surfaces, or random points spread over their area | **a lattice or random positions in its bounding box, kept where they are inside the segments** |
| Volume | its voxel centers | — | **a lattice or random positions in its extent** |
| None (the source itself) | model: **its mesh points**; volume: its voxels | model: its points, or a random subset spread over its surface | volume: **a lattice over its extent**; model: a random subset of its points, or a lattice where the field can be evaluated between them; transform: nothing |

- **Slice views:** samples in a list (points, steps along a curve, random samples on a surface or in a plane, a lattice in a plane) are shown where the slice crosses them. Samples inside a volume, regular or random, and the whole field are sampled on a lattice in the plane of the slice instead, limited to the inside of the sampling region: random samples spread through a volume would leave only the few that are within the slice thickness.
- **Region test:** the inside of the sampling region (where it is not a box) and the masking region are tested at the samples. Glyphs are only drawn where the test passes. Grid lines (and their fill) and isosurfaces carry the result from the samples they are built from, and are cut where it changes, so a grid is masked by where it was sampled, not where it was warped to. Streamlines are not cut: they are traced through the cells that the masking region shows, and stop where they leave them. The threshold limits them the same way. The cells that are left out are hidden in the image of a volume, which is traced as an image, and are not copied from a mesh.
- **Non-axis-aligned volumes:** a volume whose voxel axes are not along the RAS axes (in either direction) is traced in its own axis-aligned space: `vtkVectorFieldGeometry` rotates the vectors into that space, moves the seeds there and moves the streamlines back, with their vectors. The VTK stream tracer goes astray in a non-axis-aligned image unless it looks up the cell of every position, which takes a few times longer. Seeds, masking regions, the threshold (also by a vector component) and the streamlines that the filter gives are in the coordinates of the field as for any other volume.
- **Masking regions:** a region of interest (its box), a closed model (inside its surface), a segmentation (the inside of its segments, tested on the source representation: binary labelmap or closed surface), or a volume (its extent).

### Coloring, threshold and visibility

| Property | Glyph | Streamline | Grid | Contour |
|---|---|---|---|---|
| `ScalarVisibility`, `ActiveScalarName`, `ActiveScalarComponent`, `ColorNode` (Color by, Active scalar and its component, Color table) | solid color, or an array through the color table: any point or cell array of a model, the field for vector volumes and transforms; a multi-component array (the field too) is colored by its magnitude (`ActiveScalarComponent` -1, default) or by one of its components | same; in slice views the colors are computed per point, with the fade in the alpha | same | same; the isosurfaces are colored by the array wherever they pass |
| `ColoringSourceDisplayNode` (Same coloring as surface) | models only: takes the coloring of the surface display node (scalar visibility, array, color table, range) and follows it when it changes | same | same | same |
| `ScalarRange`, `ScalarRangeFlag` | range of the color table | same | same | same |
| `ThresholdEnabled`, `ThresholdRange` (Threshold) | hides glyphs whose colored value is out of range (applied before the placement) | limits where the streamlines are integrated, as the masking region does: they are traced through the cells of the field that have a point with the colored value in the range and cut where the value crosses a limit, so they stop where they leave the range, and seeds where the value is out of the range start nothing | — (hidden) | — (hidden; the contour levels decide which magnitudes are drawn) |
| `Visibility`, `Visibility2D`, `Visibility3D` | all views, slice views, 3D views | same | same | same |
| `Color`, `Opacity` (Opacity) | 3D solid color and opacity | same | same | same; the isosurfaces are drawn at `Opacity` × `ContourOpacity3D` |
| `SliceIntersectionOpacity`, `SliceIntersectionThickness` (Line width) | slice view opacity and line width | same | same | same |

### Differences by source

#### Vector volume

- Display nodes are only added when the user asks for one: color volumes, for example, are vector volumes too, and loading them does not cover them with glyphs. They are colored with the Plasma color table by default.
- The arrays are fixed: the field is always the voxel vectors, colored by magnitude. The vector array selector is hidden.
- The vector components are always read as RAS directions.
- Without a sampling region node, glyphs are placed on a lattice over the whole volume, or at random in it, or at its voxels.
- The color range comes from the voxel magnitudes and follows the time points of a sequence.
- In slice views, a parent transform is applied by looking up each slice position in the coordinate system of the volume.

#### Model

- The arrays are the point or cell data of the mesh. Cell data is drawn at the cell centers, which are points without cells, so a random subset of them can only be spread over their bounds, not over a surface or inside. Cell data is averaged to the points for contours and streamlines.
- The field can be colored by any point or cell array of the mesh, not only the field itself. When the field is sampled (on a lattice, at node points, in a slice plane, or from the B-spline fit), the samplers carry that array along with the vectors: they interpolate it inside the cells of a volumetric mesh, and fit it the same way as the vectors when interpolating between points. A cell array that colors a field drawn at the points is averaged onto the points.
- Glyphs are placed at the mesh points by default, or a random subset of them. In slice views, these are the points within the slice thickness.
- **Volumetric mesh:** every mode is available. Glyphs on a lattice and grids sample inside the cells exactly, and contours and streamlines use the cells of the mesh, so nothing is drawn outside the cells: a grid over a thin solid is mostly empty. With `InterpolateBetweenPoints` on, every mode is drawn from a B-spline fit to the points instead, which extends the field up to `InterpolationMaximumDistance` beyond the mesh.
- **Surface mesh or point cloud:** glyphs at its own points only, unless `InterpolateBetweenPoints` is on. Then it can be sampled at other nodes and inside itself, all modes are drawn from a B-spline fit, and the fit error is shown (`GetInterpolationResidual()`).
- **Displacements:** to warp other nodes by a displacement array of a model, or to show it as a transform, import the model into a transform node (Transforms module, "Convert", Import, Model). It fits the same kind of B-spline to the points as `InterpolateBetweenPoints`, or uses the displacements as they are, and its vector field is shown as described under [Transform](#transform). See [Compute transform](../user_guide/modules/transforms.md#compute-transform).

#### Transform

`vtkSlicerVectorFieldRenderingLogic::SetDefaultTransformDisplayProperties()` sets the defaults of a vector field display node of a transform: glyph sizes in mm, a 0.01 mm threshold, and a "Displacement to color" procedural color map from displacement magnitude in mm to color.

- The field is the displacement of the transform to world, including the parent transforms, sampled at world positions. The parent transform of the transform node is not applied a second time.
- There is no streamline mode, and the arrays are fixed.
- 3D views show nothing until a sampling region node is set; the selector shows "Select a node". Slice views need no node.
- The sampling spacing is 10 mm by default, as the glyphs of a transform were spaced before every mode shared one spacing.
- The color range is the range of the color map, not of the data.
- **By transform type:** the transform type is not checked; the displacement is computed with `GetTransformToWorld` at each sample.
  - **Linear:** the displacement varies linearly, so the contours are planes or cylinders and the grid is deformed affinely.
  - **B-spline, grid and thin-plate spline:** interpolated at each sample. Thin-plate splines and composite transforms are the slowest to evaluate.
  - **Inverse-defined:** transforms that are only stored "from parent" are inverted iteratively at every sample, so they are noticeably slower to display.
  - **Invert:** swaps the two directions, and the display is updated immediately.
