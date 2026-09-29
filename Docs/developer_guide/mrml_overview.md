# MRML Overview

## Introduction

Medical Reality Modeling Language (MRML, pronounced "MURml") is a data model developed to represent all data sets that may be used in medical software applications.

- MRML software library: An open-source software library that implements MRML data in-memory representation, reading/writing files, visualization, processing framework, and GUI widgets for viewing and editing. The library is based on the VTK toolkit, uses ITK for reading/writing some file formats, and has a few additional optional dependencies, such as Qt for GUI widgets. The library is kept fully independent from 3D Slicer and so it can be used in any other medical applications, but 3D Slicer is the only major application that uses it and therefore MRML library source code is maintained in 3D Slicer's source code repository.
- MRML file: When MRML data is saved to file, an XML document is created (with a .mrml file extension), which contains an index of all data sets and it may refer to other data files for bulk data storage. A variant of this file format is the MRML bundle file, which contains the .mrml file and all referenced data files in a single zip file (with .mrb extension).

## MRML Scene

- All data is stored in a *MRML scene*, which contains a list of *MRML nodes*.
- Each MRML node has a unique ID in the scene, has a name, custom attributes (key:value pairs), and a number of additional properties to store information specific to its data type. Node types include image volume, surface mesh, point set, transformation, etc.
- Nodes can keep *references* (links) to each other.
- Nodes can *invoke events* when their contents or internal state change. The most common event is a "Modified" event, which is invoked whenever the node content is changed. Other nodes, [application logic objects](https://www.slicer.org/wiki/Documentation/Nightly/Developers/Logics), or user interface widgets may add *observers*, which are callback functions that are executed whenever the corresponding event is invoked.

## MRML nodes

### Basic MRML node types

- **Data nodes** store basic properties of a data set. Because the same data set can be displayed in different ways (even within the same application, you may want to show the same data set differently in each view), display properties are not stored in the data node. Similarly, the same data set can be stored in various file formats, therefore file storage properties are not stored in a data node. Data nodes are typically thin wrappers over VTK objects, such as vtkPolyData, vtkImageData, vtkTable. The most important 3D Slicer core data nodes are the following:
  - **Volume** ([vtkMRMLVolume](https://apidocs.slicer.org/main/classvtkMRMLVolumeNode.html) and its subclasses): stores a 3D image. Each voxel of a volume may be a scalar (to store images with continuous grayscale values, such as a CT image), label (to store discrete labels, such as a segmentation result), vector (for storing displacement fields or RGB color images), or tensor (MRI diffusion images). 2D image volumes are represented as single-slice 3D volumes. 4D volumes are stored in sequence nodes (vtkMRMLSequenceNode).
  - **Model** ([vtkMRMLModelNode](https://apidocs.slicer.org/main/classvtkMRMLModelNode.html)): stores a surface mesh (polygonal elements, points, lines, etc.) or a volumetric mesh (tetrahedral, wedge elements, unstructured grid, etc.).
  - **Segmentation** ([vtkMRMLSegmentationNode](https://apidocs.slicer.org/main/classvtkMRMLSegmentationNode.html)): complex data node that can store an image segmentation (also known as contouring, labeling). It can store multiple representations internally; for example it can store both a binary labelmap image and a closed surface mesh.
  - **Markups** ([vtkMRMLMarkupsNode](https://apidocs.slicer.org/main/classvtkMRMLMarkupsNode.html) and subclasses): stores simple geometrical objects, such as point lists (formerly called "fiducial lists"), lines, angles, curves, planes for annotation and measurements.
  - **Transform** ([vtkMRMLTransformNode](https://apidocs.slicer.org/main/classvtkMRMLTransformNode.html)): stores a geometrical transformation that can be applied to any [transformable nodes](https://apidocs.slicer.org/main/classvtkMRMLTransformableNode.html). A transformation can contain any number of linear or non-linear (warping) transforms chained together. In general, it is recommended to use vtkMRMLTransformNode. Child types (vtkMRMLLinearTransformNode, vtkMRMLBSplineTransformNode, vtkMRMLGridTransformNode) are kept for backward compatibility and to allow filtering for specific transformation types in user interface widgets.
  - **Text** ([vtkMRMLTextNode](https://apidocs.slicer.org/main/classvtkMRMLTextNode.html)): stores text data, such as configuration files, descriptive text, etc.
  - **Table** ([vtkMRMLTableNode](https://apidocs.slicer.org/main/classvtkMRMLTableNode.html)): stores tabular data (multiple scalar or vector arrays), used mainly for showing quantitative results in tables and plots
- **Display nodes** ([vtkMRMLDisplayNode](https://apidocs.slicer.org/main/classvtkMRMLDisplayNode.html) and its subclasses) specify properties for displaying data nodes. For example, a model node's color is stored in a display node associated with a model node.
  - Multiple display nodes may be added for a single data node, each specifying different display properties and view nodes. Built-in 3D Slicer modules typically show and allow editing of only the *first* display node associated with a data node.
  - If a display node specifies a list of view nodes then the associated data node is displayed in only those views.
  - Display nodes may refer to *color nodes* to specify a list of colors or color look-up-tables.
  - When a data node is created, a default display node can be added by calling its `CreateDefaultDisplayNodes()` method. In some cases, 3D Slicer detects if the display and storage node are missing and tries to create default nodes, but developers should not rely on this error-recovery mechanism.
- **Storage nodes** ([vtkMRMLStorageNode](https://apidocs.slicer.org/main/classvtkMRMLStorageNode.html) and its subclasses) specify how to store a data node in a file. It can store one or more file names, compression options, coordinate system information, etc.
  - A default storage node may be created for a data node by calling its `CreateDefaultStorageNode()` method.
- **View nodes** ([vtkMRMLAbstractViewNode](https://apidocs.slicer.org/v4.8/classvtkMRMLAbstractViewNode.html) and subclasses) specify view layout and appearance of views, such as background color. Additional nodes related to view nodes include:
  - vtkMRMLCameraNode stores properties of camera of a 3D view.
  - vtkMRMLClipModelsNode defines how to clip models with slice planes.
  - vtkMRMLCrosshairNode stores position and display properties of a view crosshair (that is positioned by holding down `Shift` key while moving the mouse in slice or 3D views) and also provides the current mouse pointer position at any time.
  - vtkMRMLLayoutNode defines the current view layout: what views (slice, 3D, table, etc.) are displayed and where. In addition to switching between built-in view layouts, custom view layouts can be specified using an XML description.
  - vtkMRMLInteractionNode specifies an interaction mode of viewers (view/transform, window/level, place markups), such as what happens when the user clicks in a view
  - vtkMRMLSelectionNode stores global state information of the scene, such as active markup (that is being placed), units (length, time, etc.) used in the scene, etc
- **Plot nodes** specify how to display table node contents as plots. A [plot series node](https://apidocs.slicer.org/main/classvtkMRMLPlotSeriesNode.html) specifies a data series using one or two columns of a table node. A [plot chart node](https://apidocs.slicer.org/main/classvtkMRMLPlotChartNode.html) specifies which series to plot and how. A [plot view node](https://apidocs.slicer.org/main/classvtkMRMLPlotViewNode.html) specifies which plot chart to show in a view and how the user can interact with it.
- **Subject hierarchy node** ([vtkMRMLSubjectHierarchyNode](https://apidocs.slicer.org/main/classvtkMRMLSubjectHierarchyNode.html)) allows organization of data nodes into folders. Subject hierarchy folders may be associated with display nodes, which can be used to override display properties of all children in that folder. It replaces all previous hierarchy management methods, such as model or annotation hierarchies.
- [**Sequence node**](https://github.com/Slicer/Slicer/blob/main/Libs/MRML/Core/vtkMRMLSequenceNode.h) stores a list of data nodes to represent time sequences or other multidimensional data sets in the scene. A [sequence browser node](https://github.com/Slicer/Slicer/blob/main/Modules/Loadable/Sequences/MRML/vtkMRMLSequenceBrowserNode.h) specifies which one of the internal data nodes should be copied to the scene so that it can be displayed or edited. The node that represents a node of the internal scene is called a *proxy node*. When a proxy node is modified in the scene, all changes can be saved into the internal scene.

Detailed documentation of MRML API can be found [here](https://apidocs.slicer.org/main/classes.html).

### MRML node attributes

MRML nodes can store custom attributes as (attribute name and value) pairs, which allow storing additional application-specific information in nodes without the need to create new node types.

To avoid name clashes, custom modules that add attributes to nodes should prefix the attribute name with the module's name and the '.' character. Example: the DoseVolumeHistogram module can use attribute names such as `DoseVolumeHistogram.StructureSetName`, `DoseVolumeHistogram.Unit`, `DoseVolumeHistogram.LineStyle`.

MRML node attributes can also be used as filter criteria in MRML node selector widgets in the user interface.

### MRML Node References

MRML nodes can reference and observe other MRML nodes using the node reference API. A node may reference multiple nodes, each performing a distinct role, and each addressed by a unique string. The same role name can be used to reference multiple nodes.

Node references are used, for example, for linking data nodes to display and storage nodes and modules can add more node references without changing the referring or referred node.

For more details, see [this page](https://www.slicer.org/wiki/Documentation/Nightly/Developers/MRML/NodeReferences).

### MRML Events and Observers

- Changes in the MRML scene and individual nodes propagate to other observing nodes, GUI, and Logic objects via VTK events and VTK's command-observer mechanism.
- `vtkSetMacro()` automatically invokes ModifiedEvent. Additional events can be invoked using the `InvokeEvent()` method.
- Using the `AddObserver()`/`RemoveObserver()` methods is tedious and error-prone, therefore it is recommended to instead use [EventBroker](https://www.slicer.org/wiki/Slicer3:EventBroker) and the vtkObserverManager helper class, macros, and callback methods.
  - MRML observer macros are defined in Libs/MRML/vtkMRMLNode.h
  - vtkSetMRMLObjectMacro - registers a MRML node with another VTK object (another MRML node, Logic or GUI). No observers are added.
  - vtkSetAndObserveMRMLObjectMacro - registers a MRML node and adds an observer for vtkCommand::ModifiedEvent.
  - vtkSetAndObserveMRMLObjectEventsMacro - registers a MRML node and adds an observer for a specified set of events.
  - The `SetAndObserveMRMLScene()` and `SetAndObserveMRMLSceneEvents()` methods are used in GUI and Logic objects to observe Modified, NewScene, NodeAdded, etc. events.
  - The `ProcessMRMLEvents()` method should be implemented in MRML nodes, Logic, and GUI classes in order to process events from the observed nodes.

## Advanced topics

### Parameter Nodes

Parameter nodes are a specific use of MRML nodes to store parameters for a given function/module. For more information, see [Parameter Nodes](parameter_nodes/overview.md).

### Scene undo/redo

MRML Scene provides an Undo/Redo mechanism that restores a previous state of the scene and individual nodes. By default, undo/redo is disabled and not displayed on the user interface, because it increased memory usage and was not tested thoroughly.

Basic mechanism:
- Undo/redo is based on saving and restoring the state of MRML nodes in the Scene.
- A MRML scene can save a snapshot of all nodes into special Undo and Redo stacks.
- The Undo and Redo stacks store copies of nodes that have changed from the previous snapshot. The nodes that have not changed are stored by a reference (pointer).
- When an Undo is called on the scene, the current state of the Undo stack is copied into the current scene and also into the Redo stack.
- All Undoable operations must store their data as MRML nodes

The developer controls at what point the snapshot is saved by calling the `SaveStateForUndo()` method on the MRML scene. `SaveStateForUndo()` saves the state of all nodes in the scene. It should be called in GUI/Logic classes before changing the state of MRML nodes. This is usually done in the ProcessGUIEvents method that processes events from the user interactions with GUI widgets. `SaveStateForUndo()` should not be called while processing transient events such as continuous events sent by the user interface while dragging a slider (for example vtkKWScale::ScaleValueStartChangingEvent).

The following methods on the MRML scene are used to manage Undo/Redo stacks:

- `vtkMRMLScene::Undo()` restores the previously saved state of the MRML scene.
- `vtkMRMLScene::Redo()` restores the previously undone state of the MRML scene.
- `vtkMRMLScene::SetUndoOff()` ignores following SaveStateForUndo calls (useful when making multiple changes to the scene/nodes that do not need to be undoable separately).
- `vtkMRMLScene::SetUndoOn()` enables following SaveStateForUndo calls.
- `vtkMRMLScene::ClearUndoStack()` clears the undo history.
- `vtkMRMLScene::ClearRedoStack()` clears the redo history.

### File reading and writing

Files are read into the scene and written from the scene by VTK classes in MRML Logic, so that loading and saving work the same way in all applications, including those that do not use Qt:

- [`vtkMRMLFileIOManager`](https://apidocs.slicer.org/main/classvtkMRMLFileIOManager.html) keeps the list of registered readers and writers, chooses the reader or writer to use for a file or a node, and loads and saves nodes using them. The application logic owns the file IO manager of the application (`vtkMRMLApplicationLogic::GetFileIOManager()`). Everything that loads or saves files in Slicer (drag-and-drop, the Add data and Save data dialogs, `slicer.util.loadNodeFromFile`, `slicer.util.saveNode`, etc.) uses it.
- Readers ([`vtkMRMLFileReader`](https://apidocs.slicer.org/main/classvtkMRMLFileReader.html) subclasses) and writers ([`vtkMRMLFileWriter`](https://apidocs.slicer.org/main/classvtkMRMLFileWriter.html) subclasses) derive from [`vtkMRMLFileIOHandler`](https://apidocs.slicer.org/main/classvtkMRMLFileIOHandler.html). Each has a file type (such as `VolumeFile`), a description that is displayed to users (such as `Volume`), and name filters (such as `Volume (*.nrrd *.nii.gz)`).
- To choose a reader, the manager asks each reader for its confidence (between 0.0 and 1.0) that it can load the file (`CanLoadFileConfidence()`) and uses the reader with the highest confidence (readers registered earlier win ties). By default, the confidence is based on the file extension: 0.5 + 0.01 × the length of the longest matched extension, so that more specific readers (`*.seg.nrrd`) are preferred over more generic ones (`*.nrrd`). Writers are chosen similarly (`CanWriteObjectConfidence()`). Groups of files that must be loaded together (for example, when the user adds a folder) are recognized by `ExamineFileListConfidence()` of readers.
- Parameters of reading and writing, such as `fileName`, `name`, `nodeID`, and the values of reader and writer [options](#reader-and-writer-options), are passed as a [`vtkMRMLIOProperties`](https://apidocs.slicer.org/main/classvtkMRMLIOProperties.html) object. IDs of loaded or written nodes and messages to be displayed to the user (`GetUserMessages()`) are available in the reader or writer after it was used.
- Readers and writers of modules are provided by the module logic ([`vtkMRMLModuleLogic::CreateFileIOHandlers()`](#the-reader)). The application logic registers them in the file IO manager when the logic is set as the module logic and unregisters them when the module logic is replaced, removed, or deleted. Readers and writers of scripted modules are [implemented in Python](#readers-and-writers-implemented-in-python) and registered automatically. Code that registers a reader or writer in the file IO manager by other means (`Register()`, `RegisterReader()`, `RegisterWriter()`) is responsible for unregistering it (`Unregister()`).
- The file IO manager invokes events that user interfaces can observe: `HandlerRegisteredEvent` and `HandlerUnregisteredEvent` (with the reader or writer as call data), `NewFileLoadedEvent` and `FileSavedEvent` (with the properties of the loaded or saved file as call data; for loaded files it contains `fileType` and the IDs of loaded nodes in `nodeIDs`).

#### Qt interface

The Qt classes of file reading and writing are kept for backward compatibility and to provide Qt options widgets:

- `qSlicerCoreIOManager` (and its subclass `qSlicerIOManager`, which adds dialogs) forwards all calls to `vtkMRMLFileIOManager`.
- `qSlicerIO`, `qSlicerFileReader`, and `qSlicerFileWriter` are the base classes of legacy Qt-based readers and writers, which are deprecated. Qt-based readers and writers that are registered using `qSlicerCoreIOManager::registerIO()` are added to the file IO manager through an adapter that calls the methods of the Qt-based object, therefore methods that are overridden in subclasses are used.
- Qt-based code that needs a Qt object for a VTK-based reader or writer (for example, `qSlicerCoreIOManager::reader()`) gets a `qSlicerVTKFileReader` or `qSlicerVTKFileWriter`, which delegates all calls to the VTK-based reader or writer. `qSlicerNodeWriter` is a `qSlicerVTKFileWriter` that uses a `vtkMRMLNodeWriter`.
- Subclasses of `qSlicerVTKFileWriter` may override methods. If `canWriteObject()` is overridden then `canWriteObjectConfidence()` is computed from it. `qSlicerNodeWriter::write()` finds the node using `getNodeByID()` and checks it using `canWriteObjectConfidence()`, so that overrides of these methods are respected when writing, too. Other `qSlicerVTKFileWriter` subclasses that override `canWriteObject()` to accept more objects than the VTK-based writer must override `write()` as well.

### Creating Custom MRML Node Classes

If you are adding new functionality to 3D Slicer either via extensions, or even updates to the core, most of the time the existing MRML nodes will be sufficient. Many powerful C++ and Python extensions simply use and combine the existing node types to create new functionality. Instead of creating new MRML nodes from scratch, other extensions subclass from existing nodes and add just a few methods to get the needed functionality. That said, if existing MRML nodes do not offer enough (or almost enough) functionality to enable what needs to be done, it is possible to create custom MRML node classes with a little bit of effort.

There are a number of different MRML nodes and helper classes that can be implemented to enable new MRML data type functionality. Here is the not-so-short list. We will go over each of these in detail.

1. [Data node](#the-data-node)
2. [Display node](#the-display-node)
3. [Widget](#the-widget)
4. [Widget Representation](#the-widget-representation)
5. [Displayable Manager](#the-displayable-manager)
6. [Storage node](#the-storage-node)
7. [Reader](#the-reader)
8. [Writer](#the-writer)
9. [Subject Hierarchy Plugin](#the-subject-hierarchy-plugin)
10. [Module](#the-module-aka-putting-it-all-together)

While technically not all of these need to be implemented for a new MRML type to be used and useful, implementing all of them will yield the best results. The resulting MRML type will "be like the Model" and will streamline future maintenance work by providing relevant hints.

:::{note}

MRML nodes are implemented in C++.


MRML nodes can be implemented in a 3D Slicer extension.
:::

:::{note}

All links to API class and function documentation redirecting to `https://apidocs.slicer.org` correspond to documentation generated from the latest commit of the `main` branch of 3D Slicer. This means that versions of this documentation associated with an older version of 3D Slicer may be out of sync with the linked API.

:::

#### Convention

For the filenames and classes, replace `<MyCustomType>` with the name of your type.

#### The data node

The data node is where the essence of the new MRML type will live. It is where the actual data resides. Notably absent from the data node is any description of how the data should be displayed or stored on disk.

Files:

```
|-- <Extension>
       |-- <Module>
              |-- MRML
                    |-- vtkMRML<MyCustomType>Node.h
                    |-- vtkMRML<MyCustomType>Node.cxx
```

Key points:

* Naming convention for class: `vtkMRML<MyCustomType>Node`
  * E.g. [`vtkMRMLModelNode`](https://apidocs.slicer.org/main/classvtkMRMLModelNode.html)
* Inherits from [`vtkMRMLDisplayableNode`](https://apidocs.slicer.org/main/classvtkMRMLDisplayableNode.html) if it is going to be displayed in the 3D or slice views.
* Constructing a new node:
  * Declare [`vtkMRMLNode* CreateNodeInstance() override`](https://apidocs.slicer.org/main/classvtkMRMLNode.html#a6acd6b70ed94ae7384372bd5cb569639) and `static vtkMRMLYourNodeType* New()`. The implementations will be generated by using the macro [`vtkMRMLNodeNewMacro(vtkMRMLYourNodeType);`](https://apidocs.slicer.org/main/vtkMRMLNode_8h.html#acfdcb323c511216ccf8279632a8f8e94) in your cxx file.
  * Create a protected default constructor.
    * It must be protected because VTK allows its objects to be created through only the `New()` factory function.
    * Because of the use of the `New()` factory function, constructors with parameters are not typically used.
  * Create a destructor if needed.
  * Delete copy/move constructors and copy/move assignment operators.
* To save to a Medical Reality Bundle (MRB) file:
  * Override [`const char* GetNodeTagName()`](https://apidocs.slicer.org/main/classvtkMRMLNode.html#a65538ef0ba40565a13a6f42d49031592) to return a unique XML tag.
  * Override [`void ReadXMLAttributes(const char** atts)`](https://apidocs.slicer.org/main/classvtkMRMLNode.html#a7ab4e8e10cadd4486acb626763d0a891) and [`void WriteXML(ostream& of, int indent)`](https://apidocs.slicer.org/main/classvtkMRMLNode.html#aaff972a037da725aaa318bf4b8a6f9a8) to save to XML any attributes of the data node that will not be saved by the writer.
* To work with Transforms:
  * Override [`bool CanApplyNonLinearTransforms() const`](https://apidocs.slicer.org/main/classvtkMRMLTransformableNode.html#a7a7dda29c7ba59e55dcbaeb0f06c7f6b) to return true or false depending on whether non-linear transforms can be applied to your data type.
  * Override [`void OnTransformNodeReferenceChanged(vtkMRMLTransformNode* transformNode)`](https://apidocs.slicer.org/main/classvtkMRMLTransformableNode.html#aaf5e6afd30216db5aafeb7e1141e9eee). It will be called when a new transform is applied to your data node.
  * Override [`void ApplyTransform(vtkAbstractTransform* transform)`](https://apidocs.slicer.org/main/classvtkMRMLTransformableNode.html#add3fb3ed7b65c0f2de92f535f013869b), which will be called when a transform is hardened.
  * See bullet on `ProcessMRMLEvents`.
* Override [`void GetRASBounds(double bounds[6])`](https://apidocs.slicer.org/main/classvtkMRMLDisplayableNode.html#aa7fbcfe229f9249e1701bef2bbf7d64b) and [`void GetBounds(double bounds[6])`](https://apidocs.slicer.org/main/classvtkMRMLDisplayableNode.html#a0976f1f4f57c701f3301391e9b95e6f9) to allow the "Center the 3D view on scene" button in the 3D viewer to work. Note that the difference between these functions is that `GetRASBounds` returns the bounds after all transforms have been applied, while `GetBounds` returns the pre-transform bounds.
* Use macro `vtkMRMLCopyContentMacro(vtkMRMLYourNodeType)` in the class definition and implement [`void CopyContent(vtkMRMLNode* anode, bool deepCopy)`](https://apidocs.slicer.org/main/classvtkMRMLNode.html#a94940453f557fbc3a59f65bd37311cef) in the cxx file. This should copy the data content of your node via either a shallow or deep copy.
* Override [`vtkMRMLStorageNode* CreateDefaultStorageNode()`](https://apidocs.slicer.org/main/classvtkMRMLStorableNode.html#aa35937e99fbb721847c3aea7a818fa10) to return an owning pointer default storage node type for your class (see [The storage node](#the-storage-node)).
* Override [`void CreateDefaultDisplayNodes()`](https://apidocs.slicer.org/main/classvtkMRMLDisplayableNode.html#ac9f1e28f999d5ccc035317246b728b03) to create the default display nodes (for 3D and/or 2D viewing).
* Override [`void ProcessMRMLEvents(vtkObject* caller, unsigned long event, void* callData)`](https://apidocs.slicer.org/main/classvtkMRMLNode.html#afce3fc71df4f8d1e56901846f3c86a51):
  * This is used to process any events that happen regarding this object.
  * This method should handle the `vtkMRMLTransformableNode::TransformModifiedEvent` which is emitted any time the transform that is associated with the data object is changed.
* Convenience methods - while not necessarily needed, they are nice to have.
  * `Get<MyCustomType>DisplayNode()` function that returns the downcast version of [`GetDisplayNode()`](https://apidocs.slicer.org/main/classvtkMRMLDisplayableNode.html#ab2cac46d934a10817b75b37471fe65a8) saves users of your class a bit of downcasting.
* Other methods:
  * Add other methods as your heart desires to view/modify the actual content of the data being stored.

:::{tip}

Any methods with signatures that contain only primitives, raw pointers to VTK derived objects, or a few std library items like `std::vector` will be automatically wrapped for use in Python. Any functions signatures that contain other classes (custom classes, smart pointers from the std library, etc) will not be wrapped. For best results, try to use existing VTK data objects, or have your custom classes derive from [`vtkObject`](https://vtk.org/doc/nightly/html/classvtkObject.html) to get automatic wrapping.

:::

#### The display node

The display node, contrary to what one may think, is not actually how a MRML object is displayed on screen. Instead it is the list of options for displaying a MRML object. Things like colors, visibility, opacity; all of these can be found in the display node.

Files:

```
|-- <Extension>
       |-- <Module>
              |-- MRML
                    |-- vtkMRML<MyCustomType>DisplayNode.h
                    |-- vtkMRML<MyCustomType>DisplayNode.cxx
```

Key Points:

* Naming convention for class: `vtkMRML<MyCustomType>DisplayNode`
  * E.g. [`vtkMRMLModelDisplayNode`](https://apidocs.slicer.org/main/classvtkMRMLModelDisplayNode.html)
* Inherits from [`vtkMRMLDisplayNode`](https://apidocs.slicer.org/main/classvtkMRMLDisplayNode.html).
* Constructing a new node is same as the [data node](#the-data-node).
* To save to an MRB is same as for the [data node](#the-data-node):
  * Some MRML types like Markups store display information when the actual data is being stored via the writer/storage node. If you do that, no action is needed in this class.
* Convenience methods:
  * `Get<YourDataType>Node()` function that returns a downcasted version of [`vtkMRMLDisplayableNode* GetDisplayableNode()`](https://apidocs.slicer.org/main/classvtkMRMLDisplayNode.html#a7100d9379c0b96b2a9ab1ad0a8514af7).
* Other methods:
  * Add any methods regarding coloring/sizing/displaying your data node.

#### The widget

The widget is interaction half of actually putting a usable object in the 2D or 3D viewer. It is in charge of making the widget representation and interacting with it.

:::{note}

If your MRML node is display only without any interaction from the viewers, the widget is not necessary, just the [widget representation](#the-widget-representation) for displaying.

:::

Files:

```
|-- <Extension>
       |-- <Module>
              |-- VTKWidgets
                    |-- vtkSlicer<MyCustomType>Widget.h
                    |-- vtkSlicer<MyCustomType>Widget.cxx
```

Key points:

* Naming convention for class: `vtkSlicer<MyCustomType>Widget`
* Inherits from [`vtkMRMLAbstractWidget`](https://apidocs.slicer.org/main/classvtkMRMLAbstractWidget.html).
* Constructing a new node is same as the [data node](#the-data-node).
* For viewing:
  * Add function(s) to create [widget representation](#the-widget-representation)(s). These will typically take a display node, a view node, and a vtkRenderer.
  * E.g. `void CreateDefaultRepresentation(vtkMRML<MyCustomType>DisplayNode* myCustomTypeDisplayNode, vtkMRMLAbstractViewNode* viewNode, vtkRenderer* renderer);`
* For interaction override some or all of the following methods from vtkMRMLAbstractWidget:
  * [`bool CanProcessInteractionEvent(vtkMRMLInteractionEventData* eventData, double& distance2)`](https://apidocs.slicer.org/main/classvtkMRMLAbstractWidget.html#a18d9e1148c3d53ea6244c69265e6176a)
  * [`bool ProcessInteractionEvent(vtkMRMLInteractionEventData* eventData)`](https://apidocs.slicer.org/main/classvtkMRMLAbstractWidget.html#ab8d51be6df9402eb96943e0e2d0b3123)
  * [`void Leave(vtkMRMLInteractionEventData* eventData)`](https://apidocs.slicer.org/main/classvtkMRMLAbstractWidget.html#a6428fe2d100bd1f38c306d3af74802a5)
  * [`bool GetInteractive()`](https://apidocs.slicer.org/main/classvtkMRMLAbstractWidget.html#aaca79315fd1e7dce0f09e82c0b1c5e6e)
  * [`int GetMouseCursor()`](https://apidocs.slicer.org/main/classvtkMRMLAbstractWidget.html#a8a6e0f8a3b81636e58f60cebbbd6f4d7)

#### The widget representation

The widget representation is the visualization half of displaying a node on screen. This is where any data structures describing your type are turned into [`vtkActor`](https://vtk.org/doc/nightly/html/classvtkActor.html)s that can be displayed in a VTK render window.

Files:

```
|-- <Extension>
       |-- <Module>
              |-- VTKWidgets
                    |-- vtkSlicer<MyCustomType>WidgetRepresentation.h
                    |-- vtkSlicer<MyCustomType>WidgetRepresentation.cxx
```

Key Points:

* Naming convention for class: `vtkSlicer<MyCustomType>WidgetRepresentation`
* Inherits from [`vtkMRMLAbstractWidgetRepresentation`](https://apidocs.slicer.org/main/classvtkMRMLAbstractWidgetRepresentation.html).
* Constructing a new node is same as the [data node](#the-data-node).
* Override [`void UpdateFromMRML(vtkMRMLNode* caller, unsigned long event, void* callData = nullptr)`](https://apidocs.slicer.org/main/classvtkMRMLAbstractWidgetRepresentation.html#a404ce09ab17e92d203c82fee2a71b0ee) to update the widget representation when the underlying data or display nodes change.
* To make the class behave like a vtkProp override:
  * [`void GetActors(vtkPropCollection*)`](https://apidocs.slicer.org/main/classvtkMRMLAbstractWidgetRepresentation.html#ab0124c85fa2d5032a37c690d58c1ef2f)
  * [`void ReleaseGraphicsResources(vtkWindow*)`](https://apidocs.slicer.org/main/classvtkMRMLAbstractWidgetRepresentation.html#aa1949d0b26d58d83df1c0764db6d19bf)
  * [`int RenderOverlay(vtkViewport* viewport)`](https://apidocs.slicer.org/main/classvtkMRMLAbstractWidgetRepresentation.html#a51314976a613b4c7324a571ee7243cbc)
  * [`int RenderOpaqueGeometry(vtkViewport* viewport)`](https://apidocs.slicer.org/main/classvtkMRMLAbstractWidgetRepresentation.html#a14e33bf29e9272dd9fe29c7baadec730)
  * [`int RenderTranslucentPolygonalGeometry(vtkViewport* viewport)`](https://apidocs.slicer.org/main/classvtkMRMLAbstractWidgetRepresentation.html#a0d00da2fcb99df807c6884ac757e1735)
  * [`vtkTypeBool HasTranslucentPolygonalGeometry()`](https://apidocs.slicer.org/main/classvtkMRMLAbstractWidgetRepresentation.html#a16347752784cec5542751b9e6d81cf2b)

:::{important}

The points/lines/etc pulled from the data node should be post-transform, if there are any transforms applied.

:::

:::{tip}

Minimize the number of actors used for better rendering performance.

:::

#### The displayable manager

The data node, display node, widget, and widget representation are all needed pieces for data actually showing up on the screen. The displayable manager is the glue that brings all the pieces together. It monitors the MRML scene, and when data and display nodes are added or removed, it creates or destroys the corresponding widgets and widget representations.

Files:

```
|-- <Extension>
       |-- <Module>
              |-- MRMLDM
                    |-- vtkMRML<MyCustomType>DisplayableManager.h
                    |-- vtkMRML<MyCustomType>DisplayableManager.cxx
```

Key Points:

* Naming convention for class: `vtkSlicer<MyCustomType>DisplayableManager`
* Inherits from [`vtkMRMLAbstractDisplayableManager`](https://apidocs.slicer.org/main/classvtkMRMLAbstractDisplayableManager.html).
* Constructing a new node is same as the [data node](#the-data-node).
* Override [`void OnMRMLSceneNodeAdded(vtkMRMLNode* node)`](https://apidocs.slicer.org/main/classvtkMRMLAbstractLogic.html#ac0102310f18a3880f00e93b8e37210c9) to watch for if a new node of your type is added to the scene. Add an appropriate widget(s) and widget representation(s) for any display nodes.
* Override [`void OnMRMLSceneNodeRemoved(vtkMRMLNode* node)`](https://apidocs.slicer.org/main/classvtkMRMLAbstractLogic.html#a92b1149d08ec5099a9ec61c0636f37c4) to watch for if a node of your type is removed from the scene. Remove corresponding widget(s) and widget representation(s).
* Override [`void OnMRMLSceneEndImport()`](https://apidocs.slicer.org/main/classvtkMRMLAbstractLogic.html#ae826dc18ec31156f0eae3456608eb1c4) to watch for an MRB file that has finished importing.
* Override [`void OnMRMLSceneEndClose()`](https://apidocs.slicer.org/main/classvtkMRMLAbstractLogic.html#a5f8a69a47eacec0cd95857beceb8eb34) to clean up when a scene closes.
* Override [`void ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void* callData)`](https://apidocs.slicer.org/main/classvtkMRMLAbstractLogic.html#a43a759874ce32846c6dffc52e5941744) to watch for changes in the data node that would require the display to change.
* Override [`void UpdateFromMRML()`](https://apidocs.slicer.org/main/classvtkMRMLAbstractDisplayableManager.html#a0e691194d46e40d73e598ba318324388) and [`void UpdateFromMRMLScene()`](https://apidocs.slicer.org/main/classvtkMRMLAbstractLogic.html#a50caeb3a373ccd53635ae2c08bc2e63e) to bring the displayable manager in line with the MRML Scene.

#### The storage node

A storage node is responsible for reading and writing data nodes to files. A single data node type can have multiple storage node types associated with it for reading/writing different formats. A storage node will be created for both normal save/load operations for a single data node, as well as when you are saving a whole scene to an MRB.

It is common for a data node’s storage node to also write relevant values out of the display node (colors, opacity, etc) at the same time it writes the data.

:::{note}

The storage node is not sufficient in itself to allow the new data node to be saved/loaded from the normal 3D Slicer save/load facilities; the [reader](#the-reader) and [writer](#the-writer) will help with that.

:::

Files:

```
|-- <Extension>
       |-- <Module>
              |-- MRML
                    |-- vtkMRML<MyCustomType>StorageNode.h
                    |-- vtkMRML<MyCustomType>StorageNode.cxx
```

Key Points:

* Naming convention for class: `vtkMRML<MyCustomType>StorageNode`
  * If you have multiple storage nodes you may have other information in the name, such as the format that is written. E.g. `vtkMRMLMarkupsJSONStorageNode`.
* Inherits from [`vtkMRMLStorageNode`](https://apidocs.slicer.org/main/classvtkMRMLStorageNode.html).
* Constructing a new node is same as the [data node](#the-data-node).
* Override [`bool CanReadInReferenceNode(vtkMRMLNode* refNode)`](https://apidocs.slicer.org/main/classvtkMRMLStorageNode.html#af76f30cec1ccdc03bc496e5d39c784da) to allow a user to inquire at runtime if a particular node can be read in by this storage node.
* Override protected [`void InitializeSupportedReadFileTypes()`](https://apidocs.slicer.org/main/classvtkMRMLStorageNode.html#af7c838cb215a8a5b1dd87a123b3fbc81) to show what file types and extensions this storage node can read (can be more than one).
* Override protected [`void InitializeSupportedWriteFileTypes()`](https://apidocs.slicer.org/main/classvtkMRMLStorageNode.html#ab161ed9e9869cb61441659fe7c358906) to show what types and extensions this storage node can read (can be more than one).
  * It is recommended to be able to read and write the same file types within a single storage node.
* Override protected [`int ReadDataInternal(vtkMRMLNode* refNode)`](https://apidocs.slicer.org/main/classvtkMRMLStorageNode.html#a56aa52786dad2724a0a6a706b7fcf014):
  * This is called by the public [`ReadData`](https://apidocs.slicer.org/main/classvtkMRMLStorageNode.html#afeeb4c95ee3164bd1d3374c053ffb04f) method.
  * This is where the actually reading data from a file happens.
* Override protected [`int WriteDataInternal(vtkMRMLNode* refNode)`](https://apidocs.slicer.org/main/classvtkMRMLStorageNode.html#a739831f001a7cebbeb72484944e842bf):
  * This is called by the public [`WriteData`](https://apidocs.slicer.org/main/classvtkMRMLStorageNode.html#a1eb7a2b35d28175e2c490ac51cbf5baf) method.
  * This is where the actually writing data to a file happens.
* If your data node uses any coordinates (most nodes that get displayed should) it is recommended to be specific in your storage format whether the saved coordinates are RAS or LPS coordinates.
  * Having a way to allow the user to specify this is even better.
* Other methods
  * Adding a `vtkMRML<MyCustomType>Node* Create<MyCustomType>Node(const char* nodeName)` function will be convenient for implementing the writer and is also convenient for users of the storage node.

:::{tip}

If your storage node reads/writes JSON, [RapidJSON](https://rapidjson.org/index.html) is already in the superbuild and is the recommended JSON parser.

It is recommended to have your extension be `.<something>.json` where the `<something>` is related to your node type (e.g. `.mrk.json` for Markups).

:::

#### The reader

The recommended way to read a file into a MRML node is through the storage node. The reader, on the other hand, exists to interface with the loading facilities of 3D Slicer (drag and drop, as well as the button to load data into the scene). As such, the reader uses the storage node in its implementation.

Readers are VTK classes, which are registered in the file IO manager of the application logic (`vtkMRMLApplicationLogic::GetFileIOManager()`). This allows loading files the same way in all applications, including those that do not use Qt.

Files:

```
|-- <Extension>
       |-- <Module>
              |-- Logic
                     |-- vtkSlicer<MyCustomType>Reader.h
                     |-- vtkSlicer<MyCustomType>Reader.cxx
```

Key Points:

* Naming convention for class: `vtkSlicer<MyCustomType>Reader`
* Inherits from [`vtkMRMLFileReader`](https://apidocs.slicer.org/main/classvtkMRMLFileReader.html).
* In the constructor, set the file type (`SetFileType`), a short description of the types of files read (`SetDescription`), and the name filters of the files that can be read (`SetNameFilters`, for example `"My custom type (*.mct)"`).
  * The file type string can be used in conjunction with the python method [`slicer.util.loadNodeFromFile`](https://slicer.readthedocs.io/en/latest/developer_guide/slicer.html#slicer.util.loadNodeFromFile)
  * The name filters should be the same as the storage node because the reader uses the storage node.
* Override `double CanLoadFileConfidence(const std::string& filePath)` if the file extension is not enough to decide if the file can be read (for example, if the file content needs to be inspected).
* Override `bool Load(vtkMRMLIOProperties* properties)`. This is the function that actually loads the node from the file (`fileName` property) into the scene (`GetScene()`). Loaded nodes must be reported by calling `AddLoadedNodeID()`. Warnings and errors that should be displayed to the user can be added to `GetUserMessages()`.
* Override `void GetOptionsDescription(vtkMRMLIOOptionsDescription* description)` if the reader has options that the user can set (see [reader and writer options](#reader-and-writer-options)).
* Provide the reader by overriding `CreateFileIOHandlers()` in the module logic. This is a public method of [`vtkMRMLModuleLogic`](https://apidocs.slicer.org/main/classvtkMRMLModuleLogic.html), the base class of `vtkSlicerModuleLogic`, therefore most module logics already have it. It returns new instances of the readers and writers of the module. The application logic calls this method when the logic is set as the module logic (`vtkMRMLApplicationLogic::SetModuleLogic()`, which is done automatically when the module is loaded), registers the returned readers and writers in its file IO manager, and unregisters them when the module logic is removed from the application logic or deleted. Other instances of the same logic class do not provide readers and writers. The reader must only keep a weak reference to the module logic.

```cpp
class vtkSlicer<MyCustomType>Logic : public vtkSlicerModuleLogic
{
public:
  ...
  std::vector<vtkSmartPointer<vtkMRMLFileIOHandler>> CreateFileIOHandlers() override;
  ...
};

std::vector<vtkSmartPointer<vtkMRMLFileIOHandler>> vtkSlicer<MyCustomType>Logic::CreateFileIOHandlers()
{
  std::vector<vtkSmartPointer<vtkMRMLFileIOHandler>> handlers;
  vtkNew<vtkSlicer<MyCustomType>Reader> reader;
  reader->Set<MyCustomType>Logic(this);
  handlers.emplace_back(reader);
  handlers.emplace_back(vtkMRMLNodeWriter::CreateNodeWriter("MyCustomType", "MyCustomTypeFile", { "vtkMRML<MyCustomType>Node" }));
  return handlers;
}
```

:::{note}

Readers that are implemented as subclasses of `qSlicerFileReader` that override `load()` and other methods are still supported, but they are deprecated (see [Qt interface](#qt-interface)).

:::

#### Readers and writers implemented in Python

A file reader can be implemented in Python by subclassing [`slicer.vtkSlicerScriptedFileReader`](slicer.md#slicer.ScriptedFileIO.vtkSlicerScriptedFileReader) and a file writer by subclassing [`slicer.vtkSlicerScriptedFileWriter`](slicer.md#slicer.ScriptedFileIO.vtkSlicerScriptedFileWriter). These are Python subclasses of the [`vtkSlicerScriptedFileReaderBridge`](https://apidocs.slicer.org/main/classvtkSlicerScriptedFileReaderBridge.html) and [`vtkSlicerScriptedFileWriterBridge`](https://apidocs.slicer.org/main/classvtkSlicerScriptedFileWriterBridge.html) C++ classes, which call the Python methods when the reader or writer is used by the file IO manager. The methods have the same names and arguments as the methods of `vtkMRMLFileReader` and `vtkMRMLFileWriter` (properties are passed as `vtkMRMLIOProperties` objects). Methods that are not overridden use the C++ implementation (for example, `CanLoadFileConfidence` checks the file extension by default).

A scripted module can provide a reader and a writer by defining `<ModuleName>FileReader` and `<ModuleName>FileWriter` classes in the module's Python file: they are registered automatically in the file IO manager when the module is set up. Readers and writers can also be registered explicitly, by calling `RegisterReader` or `RegisterWriter` of the file IO manager (`slicer.app.applicationLogic().GetFileIOManager()`).

```python
import os
import slicer
import vtk
from slicer.i18n import tr as _


class MyModuleFileReader(slicer.vtkSlicerScriptedFileReader):

    def __init__(self):
        super().__init__()
        self.SetFileType("MyCustomTypeFile")
        self.SetDescription(_("My custom type"))
        self.SetNameFilters(["My custom type (*.mct)"])

    def CanLoadFileConfidence(self, filePath):
        # Optional. Return a value between 0.0 and 1.0.
        # If not implemented then the confidence is determined from the file extension.
        # Optionally, CanLoadFile(filePath) can be implemented instead, which returns True or False.
        return 0.55 if self.GetSupportedNameFilters(filePath) else 0.0

    def GetOptionsDescription(self, description):
        # Optional. Describe the options that the user can set (see "Reader and writer options").
        # The description is requested again whenever the user changes an option, therefore
        # default values and enabled state may depend on the file name and on other options.
        fileName = description.GetFileName()
        defaultName = os.path.splitext(os.path.basename(fileName))[0] if fileName else ""
        description.AddStringOption("name", _("Name"), _("Name of the loaded node."), defaultName)
        description.AddEnumOption("encoding", _("Encoding"), _("Text encoding of the file."), "utf-8")
        description.AddEnumChoice("encoding", "utf-8", _("UTF-8"))
        description.AddEnumChoice("encoding", "latin-1", _("Latin-1"))
        description.AddBoolOption("limitLines", _("Limit lines"), _("Only load the first lines of the file."), False)
        description.AddIntOption("maxLines", _("Maximum lines"), _("Maximum number of lines to load."), 10, 1, 10000)
        description.SetOptionEnabled("maxLines", description.GetBoolOptionValue("limitLines"))
        description.AddBoolOption("show", _("Show"), _("Show the loaded node in views."), True)

    def Load(self, properties):
        # properties is a vtkMRMLIOProperties object, containing "fileName" and the options.
        # Options may not be specified (for example, when loading using slicer.util.loadNodeFromFile
        # without properties), therefore default values must be used for missing options.
        fileName = properties.GetStringProperty("fileName")
        encoding = properties.GetStringProperty("encoding", "utf-8")
        maxLines = properties.GetIntProperty("maxLines", 10) if properties.GetBoolProperty("limitLines") else None
        node = ...  # load the file into self.GetScene()
        if not node:
            self.GetUserMessages().AddMessage(vtk.vtkCommand.ErrorEvent, f"Failed to load {fileName}")
            return False
        self.AddLoadedNodeID(node.GetID())
        return True
```

Writers describe their options the same way. The properties of the description contain the ID of the node that is written (`nodeID`), therefore default values may depend on the node:

```python
class MyModuleFileWriter(slicer.vtkSlicerScriptedFileWriter):

    def __init__(self):
        super().__init__()
        self.SetFileType("MyCustomTypeFile")
        self.SetDescription(_("My custom type"))
        self.SetNodeClassNames(["vtkMRMLTextNode"])

    def GetNameFiltersForObject(self, obj):
        return ["My custom type (*.mct)"]

    def GetOptionsDescription(self, description):
        description.AddEnumOption("lineEnding", _("Line ending"), _("Line ending characters used in the file."), "LF")
        description.AddEnumChoice("lineEnding", "LF", _("LF (Linux, macOS)"))
        description.AddEnumChoice("lineEnding", "CRLF", _("CRLF (Windows)"))
        properties = description.GetProperties()
        node = self.GetScene().GetNodeByID(properties.GetStringProperty("nodeID")) if properties else None
        defaultCategory = (node.GetAttribute("MyCategory") if node else None) or ""
        description.AddStringOption("category", _("Category"), _("Category that is stored in the file."), defaultCategory)

    def Write(self, properties):
        node = self.GetScene().GetNodeByID(properties.GetStringProperty("nodeID"))
        ...  # write the node to properties.GetStringProperty("fileName")
        self.AddWrittenNodeID(node.GetID())
        return True
```

A reader can optionally implement `ExamineFileListConfidence(fileNames, properties)` to recognize a group of files that must be loaded together (for example, when the user adds multiple files or a folder in the Add data dialog). If a group is recognized, the method removes the files of the group from the `fileNames` list (the list must be modified in place), except the archetype file, sets the `properties` that are needed for loading the group (including `fileName`, which must be set to the archetype file path), and returns the confidence (between 0.0 and 1.0) that the reader can load the group. It returns 0.0 if no group is recognized. If multiple readers recognize a group of files then the one with the highest confidence is used. Alternatively, `ExamineFileList(fileNames, properties)` can be implemented, which returns the archetype file path (or an empty string), and the confidence of loading the archetype file is used (similarly to `CanLoadFile` and `CanLoadFileConfidence`).

Notes:

* The reader or writer is kept alive by the file IO manager. While only C++ refers to it, its Python object may be released, but the Python class and the attributes of the object are restored when it is used again. Therefore, store the state of the reader or writer in attributes of `self`, and do not rely on the `id()` of the object or on Python weak references to it.
* If a Python method raises an exception or returns a value of unexpected type, then the error is logged. Methods that decide if a file or node is handled (`CanLoadFile`, `CanLoadFileConfidence`, `CanWriteObject`, `CanWriteObjectConfidence`, `ExamineFileList`, `ExamineFileListConfidence`) then report that the file or node is not handled, so that a failing reader or writer does not take precedence over other readers and writers. `Load` and `Write` report failure. For other methods, the C++ implementation of the method is used.

How it works (implementation details):

* The bridge classes do not store a reference to their Python object. The Python object refers to the C++ object, therefore a reference from the C++ object to the Python object would be a reference cycle that neither the Python garbage collector nor VTK reference counting can break, and the reader or writer would never be deleted. Instead, the bridge gets its Python object from VTK (`vtkPythonUtil::GetObjectFromPointer()`) whenever a method is called. When only C++ refers to the object, VTK keeps the Python class and attributes of the object (a "ghost") and restores them when the object is wrapped again. After the object is deleted, VTK releases the kept class and attributes when it cleans up its kept data (the next time the Python object of a VTK object that has a Python subclass or attributes is released while the VTK object still exists). They are not released in the destructor of the bridge, because calling `vtkPythonUtil::FindObject()` for a deleted object is not safe.
* When a Python method raises an exception, the traceback is logged and the Python error is cleared, because there is no Python caller to report the error to (readers and writers are called from C++), and a pending Python error would make calls of other Python readers and writers fail. A result that indicates that the file or node is not handled (or failure) is returned instead, as described above.
* Strings that are not valid UTF-8 (such as file names on Linux) are passed to Python as `bytes` (as by all VTK-wrapped methods) and they are accepted as `bytes` when Python returns them.

Complete, tested examples are available in [SlicerScriptedFileReaderWriterBridgeTest.py](https://github.com/Slicer/Slicer/blob/main/Applications/SlicerApp/Testing/Python/SlicerScriptedFileReaderWriterBridgeTest.py).

:::{note}

Readers and writers that follow the legacy convention (classes that are not subclasses of `slicer.vtkSlicerScriptedFileReader` or `slicer.vtkSlicerScriptedFileWriter`) are still supported, but deprecated. A legacy reader class is instantiated with a `parent` argument and implements `description()`, `fileType()`, `extensions()`, `load(properties)`, and optionally `canLoadFile(filePath)`, `canLoadFileConfidence(filePath)`, `getOptionsDescription(description)`, `examineFileListConfidence(fileNames, properties)`, and `examineFileList(fileNames, properties)`. Properties are passed as Python dictionaries. IDs of loaded nodes are reported by setting `self.parent.loadedNodes`, messages are added to `self.parent.userMessages()`. A legacy writer class implements `description()`, `fileType()`, `extensions(obj)`, `write(properties)`, and optionally `canWriteObject(obj)`, `canWriteObjectConfidence(obj)`, and `getOptionsDescription(description)`, and reports IDs of written nodes by setting `self.parent.writtenNodes`. See [SlicerScriptedFileReaderWriterTest.py](https://github.com/Slicer/Slicer/blob/main/Applications/SlicerApp/Testing/Python/SlicerScriptedFileReaderWriterTest.py) for an example.

Legacy classes are used via [`slicer.ScriptedFileIO.LegacyScriptedFileReader`](slicer.md#slicer.ScriptedFileIO.LegacyScriptedFileReader) and [`slicer.ScriptedFileIO.LegacyScriptedFileWriter`](slicer.md#slicer.ScriptedFileIO.LegacyScriptedFileWriter). The `parent` object behaves like the reader or writer, but it only holds a weak reference to it, to avoid a reference cycle between the reader or writer and the Python object (see [`slicer.ScriptedFileIO.ScriptedFileIOParent`](slicer.md#slicer.ScriptedFileIO.ScriptedFileIOParent)). It raises `ReferenceError` if it is used after the reader or writer is deleted.

:::

#### The writer

The writer is the companion to the reader, so, similar to the reader, it does not implement the actual writing of files, but rather it uses the storage node. Its existence is necessary to use 3D Slicer's built in saving facilities, such as the save button.

Files:

```
|-- <Extension>
       |-- <Module>
              |-- Logic
                     |-- vtkSlicer<MyCustomType>Writer.h
                     |-- vtkSlicer<MyCustomType>Writer.cxx
```

Key points:

* Naming convention for class: `vtkSlicer<MyCustomType>Writer`
* Inherits from [`vtkMRMLNodeWriter`](https://apidocs.slicer.org/main/classvtkMRMLNodeWriter.html), which writes nodes using their storage node. In many cases no subclass is needed, it is enough to return a `vtkMRMLNodeWriter` created by `vtkMRMLNodeWriter::CreateNodeWriter("MyCustomType", "MyCustomTypeFile", { "vtkMRML<MyCustomType>Node" })` from `CreateFileIOHandlers()` (see the example above).
* Override `std::vector<std::string> GetNameFiltersForObject(vtkObject* object)` to provide file formats that can be written to.
  * File extensions may be different, but don't have to be, for different data nodes that in the same hierarchy (e.g. Markups Curve and Plane could reasonably require different file extensions, but they don't).
* Override `bool Write(vtkMRMLIOProperties* properties)` to do the actual writing (by way of a storage node, of course).
* Override `void GetOptionsDescription(vtkMRMLIOOptionsDescription* description)` to add options (`vtkMRMLNodeWriter` already describes compression options).
* See the [reader](#the-reader) for information on registering the writer.

#### Reader and writer options

Readers and writers describe the options that the user can set (such as loading a volume as a labelmap) in `GetOptionsDescription()`, by calling the `Add...Option()` methods of [`vtkMRMLIOOptionsDescription`](https://apidocs.slicer.org/main/classvtkMRMLIOOptionsDescription.html) (in C++ and in Python). User interfaces display widgets for the options based on this description: they get the description by calling `FillOptionsDescription()` of the reader or writer and read the options using the `GetNthOption...()` methods of the description. In the desktop application, `qSlicerGenericIOOptionsWidget` creates the widgets in the Add data and Save data dialogs (a checkbox, spinbox, line edit, combobox, or node selector for each option). Whenever the user changes an option, the widget requests the description again with the changed values, so that the default values, enabled and visible states of other options are updated. Values that the user set are kept until the file name or the written node is changed. The description can also be serialized to JSON (`GetOptionsDescriptionJSON()`) for web or remote user interfaces; JSON is only an output format. The default values of options can be retrieved without a user interface (`GetOptionValues()`).

Each option sets one property of the reader or writer (such as `labelmap` or `colorNodeID`). Supported option types: boolean (`bool`, checkbox), integer (`int`), floating-point number (`double`, with optional minimum, maximum, and number of decimals), string (`string`), list of strings (`stringList`, edited as text with items separated by a separator character), enumeration (`enum`, one of a list of choices), and MRML node (`node`, node selector of the specified node classes). A widget hint can be specified for an option (for example, `colorTable` for color nodes). Default values may depend on the file name (`description->GetFileName()`), the node that is written (`nodeID` property), and on the values of other options (`description->GetOptionValue()`), as the description is requested again whenever the user changes an option.

The value of each option is determined in this order:

1. the value in the properties of the description (the context, such as `fileName` or `nodeID`, and the values that the user edited),
2. the value in the defaults of the description (the option defaults of the reader or writer, see `GetOptionDefaults()` below),
3. the default value specified when the option is added.

Labels and tool tips must be translated using `vtkMRMLTr()` with string literals, so that the text can be extracted for translation:

```cpp
void vtkSlicer<MyCustomType>Reader::GetOptionsDescription(vtkMRMLIOOptionsDescription* description)
{
  description->AddBoolOption("labelmap",
    vtkMRMLTr("vtkSlicer<MyCustomType>Reader", "LabelMap"),
    vtkMRMLTr("vtkSlicer<MyCustomType>Reader", "Load the volume as a labelmap."),
    description->GetFileName().find("-label") != std::string::npos);
  description->AddNodeOption("colorNodeID",
    vtkMRMLTr("vtkSlicer<MyCustomType>Reader", "Color:"),
    vtkMRMLTr("vtkSlicer<MyCustomType>Reader", "Color table node used to display the volume."),
    description->GetBoolOptionValue("labelmap") ? "vtkMRMLColorTableNodeFileGenericColors.txt" : "vtkMRMLColorTableNodeGrey",
    { "vtkMRMLColorTableNode" }, /*noneEnabled=*/false);
  description->SetOptionWidget("colorNodeID", "colorTable");
}
```

Applications can override default values of options (for example, based on user settings) by setting values in `GetOptionDefaults()` of the reader or writer.

The JSON representation of the description (`ToJSON()`, `GetOptionsDescriptionJSON()`) contains the attributes and the current value of each option:

```json
{ "options": [
  { "property": "labelmap", "type": "bool", "label": "LabelMap", "toolTip": "...", "value": false,
    "enabled": true, "visible": true },
  { "property": "colorNodeID", "type": "node", "label": "Color:", "toolTip": "...", "value": "vtkMRMLColorTableNodeGrey",
    "nodeClasses": ["vtkMRMLColorTableNode"], "noneEnabled": false, "showHidden": false, "widget": "colorTable",
    "enabled": true, "visible": true },
  { "property": "coordinateSystem", "type": "enum", "label": "Coordinate system:", "value": -1,
    "choices": [ {"value": -1, "label": "Default"}, {"value": 0, "label": "RAS"} ], ... }
]}
```

#### The subject hierarchy plugin

A convenient module in 3D Slicer is the Data module. It brings all the different data types together under one roof and offers operations such as cloning, deleting, and renaming nodes that work regardless of the node type. The Data module uses the Subject Hierarchy, which is what we need to plug into so our new node type can be seen in and modified by the Data module.

Files:

```
|-- <Extension>
       |-- <Module>
              |-- SubjectHierarchyPlugins
                    |-- qSlicerSubjectHierarchy<MyCustomType>Plugin.h
                    |-- qSlicerSubjectHierarchy<MyCustomType>Plugin.cxx
```

Key Points:

* Naming convention for class: `qSlicerSubjectHierarchy<MyCustomType>Plugin`
* Inherits from [`qSlicerSubjectHierarchyAbstractPlugin`](https://apidocs.slicer.org/main/classqSlicerSubjectHierarchyAbstractPlugin.html).
* See the [reader](#the-reader) for information on defining and constructing Qt style classes.
* Override [`double canAddNodeToSubjectHierarchy(vtkMRMLNode* node, vtkIdType parentItemID=vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID) const`](https://apidocs.slicer.org/main/classqSlicerSubjectHierarchyAbstractPlugin.html#a91df5054dd11126a05e730922b5e9e43).
  * This method is used to determine if a data node can be placed in the hierarchy using this plugin.
* Override [`double canOwnSubjectHierarchyItem(vtkIdType itemID) const`](https://apidocs.slicer.org/main/classqSlicerSubjectHierarchyAbstractPlugin.html#afe009201b32cc0a115aff0022cc1dd9f) to say if this plugin can own a particular subject hierarchy item.
* Override [`const QString roleForPlugin() const`](https://apidocs.slicer.org/main/classqSlicerSubjectHierarchyAbstractPlugin.html#a55f0d686fe8e38576bc890de95622ad5) to give the plugin’s role (most often meaning the data type the plugin can handle, e.g. Markup).
* Override [`QIcon icon(vtkIdType itemID)`](https://apidocs.slicer.org/main/classqSlicerSubjectHierarchyAbstractPlugin.html#a7a1b0b5a55a2a13d7b88f3746ea573bc) and [`QIcon visibilityIcon(int visible)`](https://apidocs.slicer.org/main/classqSlicerSubjectHierarchyAbstractPlugin.html#a5bbd27154c71e174804bd833d50ce070) to set icons for your node type.
* Override [`QString tooltip(vtkIdType itemID) const`](https://apidocs.slicer.org/main/classqSlicerSubjectHierarchyAbstractPlugin.html#a7c748f1c4437fb4f63f88324b68157ef) to set a tool tip for your node type.
* Override the following to determine what happens when a user gets/sets the node color through the subject hierarchy:
  * [`void setDisplayColor(vtkIdType itemID, QColor color, QMap<int, QVariant> terminologyMetaData)`](https://apidocs.slicer.org/main/classqSlicerSubjectHierarchyAbstractPlugin.html#ad8b6d1065e78bfc73e6dcc71fce61c42)
  * [`QColor getDisplayColor(vtkIdType itemID, QMap<int, QVariant> &terminologyMetaData) const`](https://apidocs.slicer.org/main/classqSlicerSubjectHierarchyAbstractPlugin.html#a7be968d5ad3840512686b7c8501cf847)

#### The module (aka putting it all together)

If you have used 3D Slicer for any length of time, you have probably noticed that for each type of node (or set of types as in something like markups) there is a dedicated module that is used solely for interacting with the single node type (or set of types). Examples would be the Models, Volumes, and Markups modules. These modules are useful from a user perspective and also necessary to get your new node registered everywhere it needs to be.

As these are normal 3D Slicer modules, they come in three main parts, the module, the logic, and the module widget. The recommended way to create a new module is through the [Extension Wizard](/user_guide/modules/extensionwizard.md#extension-wizard).

Files:

```
|-- <Extension>
       |-- <Module>
              |-- qSlicer<MyCustomType>Module.h
              |-- qSlicer<MyCustomType>Module.cxx
              |-- qSlicer<MyCustomType>ModuleWidget.h
              |-- qSlicer<MyCustomType>ModuleWidget.cxx
              |-- Logic
                    |-- vtkSlicer<MyCustomType>Logic.h
                    |-- vtkSlicer<MyCustomType>Logic.cxx
```

In `qSlicer<MyCustomType>Module.cxx`:

* Override the [`void setup()`](https://apidocs.slicer.org/main/classqSlicerAbstractCoreModule.html#a9ad37e756338e7226f157b4eb54b9bcd) function:
  * Register your displayable manager with the [`vtkMRMLThreeDViewDisplayableManagerFactory`](https://apidocs.slicer.org/main/classvtkMRMLThreeDViewDisplayableManagerFactory.html) and/or the [`vtkMRMLSliceViewDisplayableManagerFactory`](https://apidocs.slicer.org/main/classvtkMRMLSliceViewDisplayableManagerFactory.html).
  * Register your subject hierarchy plugin with the [`qSlicerSubjectHierarchyPluginHandler`](https://apidocs.slicer.org/main/classqSlicerSubjectHierarchyPluginHandler.html).
  * Register your reader and writer with the [`qSlicerIOManager`](https://apidocs.slicer.org/main/classqSlicerIOManager.html).
* Override the [`QStringList associatedNodeTypes() const`](https://apidocs.slicer.org/main/classqSlicerAbstractCoreModule.html#a51d7c31d4901b3a313cb53b4891961f9) function and return all the new MRML classes created (data, display, and storage nodes).

In `vtkSlicer<MyCustomType>Logic.cxx`:

* Override the protected [`void RegisterNodes()`](https://apidocs.slicer.org/main/classvtkMRMLAbstractLogic.html#acfa7f65f53d5fbe6d056e7cf32a23058) function and register all the new MRML classes created (data, display, and storage nodes) with the MRML scene.

In `qSlicer<MyCustomType>ModuleWidget.cxx`:

* Override [`bool setEditedNode(vtkMRMLNode* node, QString role = QString(), QString context = QString())`](https://apidocs.slicer.org/main/classqSlicerAbstractModuleWidget.html#a52fe94bcc034b4d841d7fc05a4899d3c) and [`double nodeEditable(vtkMRMLNode* node)`](https://apidocs.slicer.org/main/classqSlicerAbstractModuleWidget.html#adebae2ce9686043d7f9dee09620b28bb) if you want this module to be connected to the Data module’s "Edit properties..." option in the right click menu.

### Slice view pipeline

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/slice_view_pipeline.png)

Another view of [VTK/MRML pipeline for the 2D slice views](https://www.slicer.org/wiki/File:SliceView.pptx).

Notes: the MapToWindowLevelColors has no lookup table set, so it maps the scalar volume data to 0,255 with no "color" operation. This is controlled by the Window/Level settings of the volume display node. The MapToColors applies the current lookup table to go from 0-255 to full RGBA.

Management of slice views is distributed between several objects:
- Slice node ([vtkMRMLSliceNode](https://apidocs.slicer.org/main/classvtkMRMLSliceNode.html)): Stores the position, orientation, and size of the slice. This is a [view node](https://apidocs.slicer.org/main/classvtkMRMLAbstractViewNode.html) and as such it stores common view properties, such as the view name, layout color, background color.
- Slice display node ([vtkMRMLSliceDisplayNode](https://apidocs.slicer.org/main/classvtkMRMLSliceDisplayNode.html)): Specifies how the slice should be displayed, such as visibility and style of display of intersecting slices. The class is based on [classvtkMRMLModelDisplayNode](https://apidocs.slicer.org/main/classvtkMRMLModelDisplayNode.html), therefore it also specifies which 3D views the slice is displayed in.
- Slice composite node ([vtkMRMLSliceCompositeNode](https://apidocs.slicer.org/main/classvtkMRMLSliceCompositeNode.html)): Specifies what volumes are displayed in the slice and how to blend them. It is ended up being a separate node probably because it is somewhat a mix between a data node (that tells what data to display) and a display node (that specifies how the data is displayed).
- Slice model node ([vtkMRMLModelNode](https://apidocs.slicer.org/main/classvtkMRMLModelNode.html)): It is a model node that displays the slice in 3D views. It stores a simple rectangle mesh on that the image cross-section is rendered as a texture.
- Slice model transform node ([vtkMRMLTransformNode](https://apidocs.slicer.org/main/classvtkMRMLTransformNode.html)). The transform node is used for positioning the slice model node in 3D views. It is faster to use a transform node to position the plane than modifying the plane points each time the slice is moved.
- Slice logic ([vtkMRMLSliceLogic](https://apidocs.slicer.org/main/classvtkMRMLSliceLogic.html)): This is not a MRML node but a logic class, which implements operationson MRML nodes. There is one logic for each slice view. The object keeps reference to all MRML nodes, so it is a good starting point for accessing any data related to slices and performing operations on slices. Slice logics are stored in the application logic object and can be retrieved like this: `sliceLogic = slicer.app.applicationLogic().GetSliceLogicByLayoutName('Red')`. There are a few other `GetSlicerLogic...` methods to get slice logic based on slice node, slice model display node, and to get all the slice logics.
- Slice layer logic([vtkMRMLSliceLayerLogic](https://apidocs.slicer.org/main/classvtkMRMLSliceLayerLogic.html)): Implements reslicing and interpolation for a volume. There is one slice layer logic for each volume layer (foreground, background, label) for each slice view.
- Slice link logic ([vtkMRMLSliceLinkLogic](https://apidocs.slicer.org/main/classvtkMRMLSliceLinkLogic.html)): There is only a single instance of this object in the entire application. This object synchronizes slice view property changes between all slice views in the same view group.

### Layout

A layout manager ([qSlicerLayoutManager][qSlicerLayoutManager-apidoc]) shows or hides layouts:

- It instantiates, shows or hides relevant view widgets.
- It is associated with a [vtkMRMLLayoutNode][vtkMRMLLayoutNode-apidoc] describing the current layout configuration and ensuring it can be saved and restored.
- It owns an instance of [vtkMRMLLayoutLogic][vtkMRMLLayoutLogic-apidoc] that controls the layout node and the view nodes in a MRML scene.
- Pre-defined layouts are described using XML and are registered in `vtkMRMLLayoutLogic::AddDefaultLayouts()`.
- Developer may register additional layout.

[qSlicerLayoutManager-apidoc]: https://apidocs.slicer.org/main/classqSlicerLayoutManager.html
[vtkMRMLLayoutNode-apidoc]: https://apidocs.slicer.org/main/classvtkMRMLLayoutNode.html
[vtkMRMLLayoutLogic-apidoc]: https://apidocs.slicer.org/main/classvtkMRMLLayoutLogic.html

#### Registering a custom layout

See [example in the script repository](script_repository.md#customize-view-layout).

#### Layout XML Format

Layout description may be validated using the following DTD:

```
<!DOCTYPE layout SYSTEM "https://slicer.org/layout.dtd"
[
<!ELEMENT layout (item+)>
<!ELEMENT item (layout*, view)>
<!ELEMENT view (property*)>
<!ELEMENT property (#PCDATA)>

<!ATTLIST layout
type (horizontal|grid|tab|vertical) #IMPLIED "horizontal"
split (true|false) #IMPLIED "true" >

<!ATTLIST item
multiple (true|false) #IMPLIED "false"
splitSize CDATA #IMPLIED "0"
row CDATA #IMPLIED "0"
column CDATA #IMPLIED "0"
rowspan CDATA #IMPLIED "1"
colspan CDATA #IMPLIED "1"
>

<!ATTLIST view
class CDATA #REQUIRED
singletontag CDATA #IMPLIED
horizontalStretch CDATA #IMPLIED "-1"
verticalStretch CDATA #IMPLIED "-1" >

<!ATTLIST property
name CDATA #REQUIRED
action (default|relayout) #REQUIRED >

]>
```

Notes:

- `layout` element:
  - `split` attribute applies only to layout of type `horizontal` and `vertical`
- `item` element:
  - `row`, `column`, `rowspan` and `colspan` attributes applies only to layout of type `grid`
  - `splitSize` must be specified only for `layout` element with `split` attribute set to `true`
- `view` element:
  - `class` must correspond to a MRML view node class name (e.g `vtkMRMLViewNode`, `vtkMRMLSliceNode` or `vtkMRMLPlotViewNode`)
  - `singletontag` must always be specified when `multiple` attribute of `item` element is specified.
- `property` element:
  - `name` attribute may be set to the following values:
    - `viewlabel`
    - `viewcolor`
    - `viewgroup`
    - `orientation` applies only if parent `view` element is associated with `class` (or subclass) of type `vtkMRMLSliceNode`
