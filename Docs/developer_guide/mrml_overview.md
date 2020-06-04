# MRML Overview

## Introduction

Medical Reality Modeling Language (MRML) is a data model developed to represent all data sets that may be used in medical software applications.

- MRML software library: An open-source software library implements MRML data in-memory representation, reading/writing files, visualization, processing framework, and GUI widgets for viewing and editing. The library is based on VTK toolkit, uses ITK for reading/writing some file format, and has a few additional optional dependencies, such as Qt for GUI widgets. The library kept fully independent from Slicer and so it can be used in any other medical applications, but Slicer is the only major application that uses it and therefore MRML library source code is maintained in Slicer's source code repository.
- MRML file: When an MRML data is saved to file then an XML document is created (with .mrml file extension), which contains an index of all data sets and it may refer to other data files for bulk data storage. A variant of this file format is the MRML bundle file, which contains the .mrml file and all referenced data files in a single zip file (with .mrb extension).

## MRML Scene

- All data is stored in a *MRML scene*, which contains a list of *MRML nodes*.
- Each MRML node has a unique ID in the scene, has a name, custom attributes (key:value pairs), and a number of additional properties to store information specific to its data type. Node types include image volume, surface mesh, point set, transformation, etc.
- Nodes can keep *references* (links) to each other.
- Nodes can *invoke events* when their contents or internal state change. Most common event is "Modified" event, which is invoked whenever the node content is changed. Other nodes, [application logic objects](https://www.slicer.org/wiki/Documentation/Nightly/Developers/Logics), or user interface widgets may add *observers*, which are callback functions that are executed whenever the corresponding event is invoked.

## MRML nodes

### Basic MRML node types

- **Data nodes** store basic properties of a data set. Since the same data set can be displayed in different ways (even within the same application, you may want to show the same data set differently in each view), display properties are not stored in the data node. Similarly, the same data set can be stored in various file formats, therefore file storage properties are not stored in a data node. Data nodes are typically thin wrappers over VTK objects, such as vtkPolyData, vtkImageData, vtkTable. Most important Slicer core data nodes are the followings:
  - **Volume** ([vtkMRMLVolume](http://apidocs.slicer.org/master/classvtkMRMLVolumeNode.html) and its subclasses): stores a 3D image. Each voxel of a volume may be a scalar (to store images with continuous grayscale values, such as a CT image), label (to store discrete labels, such as a segmentation result), vector (for storing displacement fields or RGB color images), or tensor (MRI diffusion images). 2D image volumes are represented as single-slice 3D volumes. 4D volumes are stored in sequence nodes (vtkMRMLSequenceNode).
  - **Model** ([vtkMRMLModelNode](http://apidocs.slicer.org/master/classvtkMRMLModelNode.html)): stores a surface mesh (polygonal elements, points, lines, etc.) or volumetric mesh (tetrahedral, wedge elements, unstructured grid, etc.)
  - **Segmentation** ([vtkMRMLSegmentationNode](http://apidocs.slicer.org/master/classvtkMRMLSegmentationNode.html)): complex data node that can store image segmentation (also known as contouring, labeling). It can store multiple representations internally, for example it can store both binary labelmap image and closed surface mesh.
  - **Markups** ([vtkMRMLMarkupsNode](http://apidocs.slicer.org/master/classvtkMRMLMarkupsNode.html) and subclasses): store simple geometrical objects, such as point sets, lines, angles, curves, planes for annotation and measurements. Points are called "fiducials" in Slicer terminology as they are individually identified, named points, similar to "fiducial markers" that are usedfor identifying points in radiology images. Annotations module is the old generation of markups functionality and is being phased out.
  - **Transform** ([vtkMRMLTransformNode](http://apidocs.slicer.org/master/classvtkMRMLTransformNode.html)): stores geometrical transformation that can be applied to any [transformable nodes](http://apidocs.slicer.org/master/classvtkMRMLTransformableNode.html). A transformation can contain any number of linear or non-linear (warping) transforms chained together. In general, it is recommended to use vtkMRMLTransformNode. Child types (vtkMRMLLinearTransformNode, vtkMRMLBSplineTransformNode, vtkMRMLGridTransformNode) are kept for backward compatibility and to allow filtering for specific transformation types in user interface widgets.
  - **Text** ([vtkMRMLTextNode](http://apidocs.slicer.org/master/classvtkMRMLTextNode.html)): stores text data, such as configuration files, descriptive text, etc.
  - **Table** ([vtkMRMLTableNode](http://apidocs.slicer.org/master/classvtkMRMLTableNode.html)): stores tabular data (multiple scalar or vector arrays), used mainly for showing quantitative results in tables and plots
- **Display nodes** ([vtkMRMLDisplayNode](http://apidocs.slicer.org/master/classvtkMRMLDisplayNode.html) and its subclasses) specify properties how to display data nodes. For example, a model node's color is stored in a display node associated with a model node.
  - Multiple display nodes may be added for a single data, each specifying different display properties and view nodes. Built-in Slicer modules typically only show and allow editing of the *first* display node associated with a data node.
  - If a display node specifies a list of view nodes then the associated data node is only displayed in those views.
  - Display nodes may refer to *color nodes* to specify a list of colors or color look-up-tables.
  - When a data node is created then default display node can be added by calling its `CreateDefaultDisplayNodes()` method. In some cases, Slicer detects if the display and storage node is missing and tries to create a default nodes, but the developers should not rely on this error-recovery mechanism.
- **Storage nodes** ([vtkMRMLStorageNode](http://apidocs.slicer.org/master/classvtkMRMLStorageNode.html) and its subclasses) specify how to store a data node in file. It can store one or more file name, compression options, coordinate system information, etc.
  - Default storage node may be created for a data node by calling its `CreateDefaultStorageNode()` method.
- **View nodes** ([vtkMRMLAbstractViewNode](https://apidocs.slicer.org/v4.8/classvtkMRMLAbstractViewNode.html) and subclasses) specify view layout and appearance of views, such as background color. Additional nodes related to view nodes include:
  - vtkMRMLCameraNode stores properties of camera of a 3D view.
  - vtkMRMLClipModelsNode defines how to clip models with slice planes.
  - vtkMRMLCrosshairNode stores position and display properties of view crosshair (that is positioned by holding down `Shift` key while moving the mouse in slice or 3D views) and also provides the current mouse pointer position at any time.
  - vtkMRMLLayoutNode defines the current view layout: what views (slice, 3D, table, etc.) are display and where. In addition to switching between built-in view layouts, custom view layouts can be specified using an XML description.
  - vtkMRMLInteractionNode specifies interaction mode of viewers (view/transform, window/level, place markups), such as what happens the user clicks in a view
  - vtkMRMLSelectionNode stores global state information of the scene, such as active markup (that is being placed), units (length, time, etc.) used in the scene, etc
- **Plot nodes** specify how to display table node contents as plots. [Plot series node](http://apidocs.slicer.org/master/classvtkMRMLPlotSeriesNode.html) specifies a data series using one or two columns of a table node. [Plot chart node](http://apidocs.slicer.org/master/classvtkMRMLPlotChartNode.html) specifies which series to plot and how. [Plot view node](http://apidocs.slicer.org/master/classvtkMRMLPlotViewNode.html) specifies which plot chart to show in a view and how user can interact with it.
- **Subject hierarchy node** ([vtkMRMLSubjectHierarchyNode](http://apidocs.slicer.org/master/classvtkMRMLSubjectHierarchyNode.html)) allows organization of data nodes into folders. Subject hierarchy folders may be associated with display nodes, which can be used to override display properties of all children in that folder. It replaces all previous hierarchy management methods, such as model or annotation hierarchies.
- [**Sequence node**](https://github.com/Slicer/Slicer/blob/master/Libs/MRML/Core/vtkMRMLSequenceNode.h) stores a list of data nodes to represent time sequences or other multidimensional data sets in the scene. [Sequence browser node](https://github.com/Slicer/Slicer/blob/master/Modules/Loadable/Sequences/MRML/vtkMRMLSequenceBrowserNode.h) specifies which one of the internal data node should be copied to the scene so that it can be displayed or edited. The node that represents a node of the internal scene is called a *proxy node*. When a proxy node is modified in the scene, all changes can be saved into the internal scene.

Detailed documentation of MRML API can be found in [here](http://apidocs.slicer.org/master/classes.html).

### MRML node attributes

MRML nodes can store custom attributes as (attribute name and value) pairs, which allow storing additional application-specific information in nodes without the need to create new node types.

To avoid name clashes custom modules that adds attributes to nodes should prefix the attribute name with the module's name and the '.' character. Example: the DoseVolumeHistogram module can use attribute names such as `DoseVolumeHistogram.StructureSetName`, `DoseVolumeHistogram.Unit`, `DoseVolumeHistogram.LineStyle`.

MRML nodes attributes can be also used filter criteria in MRML node selector widgets in the user interface.

### MRML Node References

MRML nodes can reference and observe other MRML nodes using the node reference API. A node may reference multiple nodes, each performing a distinct role, and each addressed by a unique string. The same role name can be used to reference multiple nodes.

Node references are used for example for linking data nodes to display and storage nodes and modules can add more node references without changing the referring or referred node.

For more details, see [this page](https://www.slicer.org/wiki/Documentation/Nightly/Developers/MRML/NodeReferences).

### MRML Events and Observers

- Changes in MRML scene and individual nodes propagate to other observing nodes, GUI and Logic objects via VTK events and command-observer mechanism.
- `vtkSetMacro()` automatically invokes ModifiedEvent. Additional events can be invoked using `InvokeEvent()` method.
- Using `AddObserver()`/`RemoveObserver()` methods is tedious and error-prone, therefore it is recommended to use [EventBroker](https://www.slicer.org/wiki/Slicer3:EventBroker) and vtkObserverManager helper class, macros, and callback methods instead.
  - MRML observer macros are defined in Libs/MRML/vtkMRMLNode.h
  - vtkSetMRMLObjectMacro - registers MRML node with another vtk object (another MRML node, Logic or GUI). No observers added.
  - vtkSetAndObserveMRMLObjectMacro - registers MRML node and adds an observer for vtkCommand::ModifyEvent. 
  - vtkSetAndObserveMRMLObjectEventsMacro - registers MRML node and adds an observer for a specified set of events. 
  - `SetAndObserveMRMLScene()` and `SetAndObserveMRMLSceneEvents()` methods are used in GUI and Logic to observe Modify, NewScene, NodeAdded, etc. events.
  - `ProcessMRMLEvents()` method should be implemented in MRML nodes, Logic, and GUI classes in order to process events from the observed nodes.

## Advanced topics

### Scene undo/redo

MRML Scene provides Undo/Redo mechanism that restores a previous state of the scene and individual nodes. By default, undo/redo is disabled and not displayed on the user interface, as it increased memory usage and was not tested thoroughly.

Basic mechanism:
- Undo/redo is based on saving and restoring the state of MRML nodes in the Scene. 
- MRML scene can save snapshot of all nodes into a special Undo and Redo stacks. 
- The Undo and Redo stacks store copies of nodes that have changed from the previous snapshot. The node that have not changed are stored by a reference (pointer). 
- When an Undo is called on the scene, the current state of Undo stack is copied into the current scene and also into Redo stack. 
- All Undoable operations must store their data as MRML nodes

Developer controls at what point the snapshot is saved by calling `SaveStateForUndo()` method on the MRML scene. `SaveStateForUndo()` saves the state of all nodes in the scene. Iy should be called in GUI/Logic classes before changing the state of MRML nodes. This is usually done in the ProcessGUIEvents method that processes events from the user interactions with GUI widgets. `SaveStateForUndo()` should not be called while processing transient events such as continuous events sent by KW UI while dragging a slider (for example vtkKWScale::ScaleValueStartChangingEvent). 

The following methods on the MRML scene are used to manage Undo/Redo stacks:

- `vtkMRMLScene::Undo()` restores the previously saved state of the MRML scene.
- `vtkMRMLScene::Redo()` restores the previously undone state of the MRML scene.
- `vtkMRMLScene::SetUndoOff()` ignores following SaveStateForUndo calls (usefull when making multiple changes to the scene/nodes that does not need to be undone). 
- `vtkMRMLScene::SetUndoOn()` enables following SaveStateForUndo calls.
- `vtkMRMLScene::ClearUndoStack()` clears the undo history.
- `vtkMRMLScene::ClearRedoStack()` clears the redo history.

### Creating Custom MRML Node Classes

- Custom MRML nodes provide persistent storage for the module parameters. 
- Custom MRML nodes should be registered with the MRML scene using `RegisterNodeClass()` so they can be saved and restored from a scene file. 
- Classes should implement the following methods: 
  - `CreateNodeInstance()` similar to VTK New() method only not static. 
  - `GetNodeTagName()` returns a unique XML tag for this node. 
  - `ReadXMLAttributes()` reads node attributes from XML file as name-value pairs. 
  - `WriteXML()` writes node attributes to output stream.
  - `Copy()` – copies node attributes.

### Slice view pipeline

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/slice_view_pipeline.png)

Another view of [VTK/MRML pipeline for the 2D slice views](https://www.slicer.org/wiki/File:SliceView.pptx).

Notes: the MapToWindowLevelColors has no lookup table set, so it maps the scalar volume data to 0,255 with no "color" operation.  This is controlled by the Window/Level settings of the volume display node.  The MapToColors applies the current lookup table to go from 0-255 to full RGBA.

