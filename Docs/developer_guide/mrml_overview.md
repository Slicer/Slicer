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

- **Data nodes** store basic properties of a data set. Since the same data set can be displayed in different ways (even within the same application, you may want to show the same data set differently in each view), display properties are not stored in the data node. Similarly, the same data set can be stored in various file formats, therefore file storage properties are not stored in a data node. Data nodes are typically thin wrappers over VTK objects, such as vtkPolyData, vtkImageData, vtkTable. Most important Slicer core data nodes are the following:
  - **Volume** ([vtkMRMLVolume](http://apidocs.slicer.org/master/classvtkMRMLVolumeNode.html) and its subclasses): stores a 3D image. Each voxel of a volume may be a scalar (to store images with continuous grayscale values, such as a CT image), label (to store discrete labels, such as a segmentation result), vector (for storing displacement fields or RGB color images), or tensor (MRI diffusion images). 2D image volumes are represented as single-slice 3D volumes. 4D volumes are stored in sequence nodes (vtkMRMLSequenceNode).
  - **Model** ([vtkMRMLModelNode](http://apidocs.slicer.org/master/classvtkMRMLModelNode.html)): stores a surface mesh (polygonal elements, points, lines, etc.) or volumetric mesh (tetrahedral, wedge elements, unstructured grid, etc.)
  - **Segmentation** ([vtkMRMLSegmentationNode](http://apidocs.slicer.org/master/classvtkMRMLSegmentationNode.html)): complex data node that can store image segmentation (also known as contouring, labeling). It can store multiple representations internally, for example it can store both binary labelmap image and closed surface mesh.
  - **Markups** ([vtkMRMLMarkupsNode](http://apidocs.slicer.org/master/classvtkMRMLMarkupsNode.html) and subclasses): store simple geometrical objects, such as point lists (formerly called "fiducial lists"), lines, angles, curves, planes for annotation and measurements. Annotations module is the old generation of markups functionality and is being phased out.
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
- `vtkMRMLScene::SetUndoOff()` ignores following SaveStateForUndo calls (useful when making multiple changes to the scene/nodes that does not need to be undone).
- `vtkMRMLScene::SetUndoOn()` enables following SaveStateForUndo calls.
- `vtkMRMLScene::ClearUndoStack()` clears the undo history.
- `vtkMRMLScene::ClearRedoStack()` clears the redo history.

### Creating Custom MRML Node Classes

There are a number of different MRML nodes and helper classes that need to be implemented for a single new MRML data type to work. Here is the not-so-short list. We will go over each of these in detail. Note that all of these can be implemented an extension and must be implemented in C++.

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

While technically not all of these need to be implemented for a new MRML type to be used and useful, implementing all of them will yield the best results. The resulting MRML type will “be like the Model”.

For the filenames and classes, replace \<MyCustomType> with the name of your type.

#### The data node

The data node is where the essence of the new MRML type will live. It is where the actual data resides. Notably absent from the data node is any description of how the data should be displayed or stored on disk (with the exception of MRB files).

Files:

* \<Extension>/\<Module>/MRML/vtkMRML\<MyCustomType>Node.h
* \<Extension>/\<Module>/MRML/vtkMRML\<MyCustomType>Node.cxx

Key points:

* Naming convention for class: `vtkMRML<MyCustomType>Node`
  * E.g. `vtkMRMLModelNode`
* Inherits from `vtkMRMLDisplayableNode` if it is going to be displayed in the 3D or slice views.
* Constructing a new node:
  * Declare `vtkMRMLNode* CreateNodeInstance() override` and `static vtkMRMLYourNodeType *New()`. The implementations will be generated by using the macro `vtkMRMLNodeNewMacro(vtkMRMLYourNodeType);` in your cxx file.
  * Create a protected default constructor.
    * It must be protected because VTK only allows its objects to be created through the `New()` factory function.
    * Because of the use of the `New()` factory function, constructors with parameters are not typically used.
  * Create a destructor if needed.
  * Delete copy/move constructors and copy/move assignment operators.
* To save to an MRB:
  * Override `const char* GetNodeTagName()` to return a unique XML tag.
  * Override `void ReadXMLAttributes(const char** atts)` and `void WriteXML(ostream& of, int indent)` to save to XML any attributes of the data node that will not be saved by the writer.
* To work with Transforms:
  * Override `bool CanApplyNonLinearTransforms() const` to return true or false depending on if non linear transforms can be applied to your data type.
  * Override `void OnTransformNodeReferenceChanged(vtkMRMLTransformNode* transformNode)`. It will be called when a new transform is applied to your data node.
  * Override `void ApplyTransform(vtkAbstractTransform* transform)`, which will be called when a transform is hardened.
  * See bullet on `ProcessMRMLEvents`.
* Override `void GetRASBounds(double bounds[6])` and `void GetBounds(double bounds[6])` to allow the “Center the 3D view on scene” button in the 3D viewer to work. Note that the difference between these functions is that `GetRASBounds` returns the bounds after all transforms have been applied, while `GetBounds` returns the pre-transform bounds.
* Use macro `vtkMRMLCopyContentMacro(vtkMRMLYourNodeType)` in the class definition and implement `void CopyContent(vtkMRMLNode* anode, bool deepCopy)` in the cxx file. This should copy the data content of your node via either a shallow or deep copy.
* Override `vtkMRMLStorageNode* CreateDefaultStorageNode()` to return an owning pointer default storage node type for your class (see “The storage node”).
* Override `void CreateDefaultDisplayNodes()` to create the default display nodes (for 3D and/or 2D viewing).
* Override `void ProcessMRMLEvents(vtkObject * caller, unsigned long event, void* callData)`:
  * This is used to process any events that happen regarding this object.
  * This method should handle the `vtkMRMLTransformableNode::TransformModifiedEvent` which is emitted any time the transform that is associated with the data object is changed.
* Convenience methods - while not necessarily needed, they are nice to have.
  * `Get<MyCustomType>DisplayNode()` function that returns the downcast version of `GetDisplayNode()` saves users of your class a bit of downcasting.
* Other methods:
  * Add other methods as your heart desires to view/modify the actual content of the data being stored.
  * Pro tip: Any methods with signatures that contain only primitives, raw pointers to VTK derived objects, or a few std library items like `std::vector` will be automatically wrapped for use in Python. Any functions signatures that contain other classes (custom classes, smart pointers from the std library, etc) will not be wrapped.

#### The display node

The display node, contrary to what one may think, is not actually how a MRML object is displayed on screen. Instead it is the list of options for displaying a MRML object. Things like colors, visibility, opacity; all of these can be found in the display node.

Files:

* \<Extension>/\<Module>/MRML/vtkMRML\<MyCustomType>DisplayNode.h
* \<Extension>/\<Module>/MRML/vtkMRML\<MyCustomType>DisplayNode.cxx

Key Points:

* Naming convention for class: `vtkMRML<MyCustomType>DisplayNode`
  * E.g. `vtkMRMLModelDisplayNode`
* Inherits from `vtkMRMLDisplayNode`.
* Constructing a new node is same as the data node.
* To save to an MRB is same as for the data node:
  * Note: some MRML types like Markups store display information when the actual data is being stored via the writer/storage node. If you do that, no action is needed in this class.
* Convenience methods:
  * `Get<YourDataType>Node()` function that returns a downcasted version of `vtkMRMLDisplayableNode* GetDisplayableNode()`.
* Other methods:
  * Add any methods regarding coloring/sizing/displaying your data node.

#### The widget

The widget is half actually putting a usable object in the 2D or 3D viewer. It is in charge of making the widget representation and interacting with it.

Files:

* \<Extension>/\<Module>/VTKWidgets/vtkSlicer\<MyCustomType>Widget.h
* \<Extension>/\<Module>/VTKWidgets/vtkSlicer\<MyCustomType>Widget.cxx

Key points:

* Naming convention for class: `vtkSlicer<MyCustomType>Widget`
* Inherits from `vtkMRMLAbstractWidget`.
* Constructing a new node is same as the data node.
* For viewing:
  * Add function(s) to create widget representation(s). These will typically take a display node, a view node, and a vtkRenderer.
  * E.g. `void CreateDefaultRepresentation(vtkMRML<MyCustomType>DisplayNode* myCustomTypeDisplayNode, vtkMRMLAbstractViewNode* viewNode, vtkRenderer* renderer);`
* For interaction override some or all of the following methods from vtkMRMLAbstractWidget:
  * `bool CanProcessInteractionEvent(vtkMRMLInteractionEventData* eventData, double &distance2)`
  * `bool ProcessInteractionEvent(vtkMRMLInteractionEventData* eventData)`
  * `void Leave(vtkMRMLInteractionEventData* eventData)`
  * `bool GetInteractive()`
  * `int GetMouseCursor()`

#### The widget representation

The widget representation is the other half of displaying a node on screen. This is where any data structures describing your type are turned into `vtkActor`s that can be displayed in a VTK render window.

Files:

* \<Extension>/\<Module>/VTKWidgets/vtkSlicer\<MyCustomType>WidgetRepresentation.h
* \<Extension>/\<Module>/VTKWidgets/vtkSlicer\<MyCustomType>WidgetRepresentation.cxx

Key Points:

* Naming convention for class: `vtkSlicer<MyCustomType>WidgetRepresentation`
* Inherits from `vtkMRMLAbstractWidgetRepresentation`.
* Constructing a new node is same as the data node.
* Override `void UpdateFromMRML(vtkMRMLNode* caller, unsigned long event, void *callData = nullptr)` to update the widget representation when the underlying data or display nodes change.
* To make the class behave like a vtkProp override:
  * `void GetActors(vtkPropCollection*)`
  * `void ReleaseGraphicsResources(vtkWindow*)`
  * `int RenderOverlay(vtkViewport* viewport)`
  * `int RenderOpaqueGeometry(vtkViewport* viewport)`
  * `int RenderTranslucentPolygonalGeometry(vtkViewport* viewport)`
  * `vtkTypeBool HasTranslucentPolygonalGeometry()`
* Notes:
  * The points/lines/etc pulled from the data node should be post-transform, if there are any transforms applied.
  * Minimize the number of actors for better rendering performance.

#### The displayable manager

The data node, display node, widget, and widget representation are all needed pieces for data actually showing up on the screen. The displayable manager is the glue that brings all the pieces together. It monitors the MRML scene, and when data and display nodes are added or removed, it creates or destroys the corresponding widgets and widget representations.

Files:

* \<Extension>/\<Module>/MRMLDM/vtkMRML\<MyCustomType>DisplayableManager.h
* \<Extension>/\<Module>/MRMLDM/vtkMRML\<MyCustomType>DisplayableManager.cxx

Key Points:

* Naming convention for class: `vtkSlicer<MyCustomType>DisplayableManager`
* Inherits from `vtkMRMLAbstractDisplayableManager`.
* Constructing a new node is same as the data node.
* Override `void OnMRMLSceneNodeAdded(vtkMRMLNode* node)` to watch for if a new node of your type is added to the scene. Add an appropriate widget(s) and widget representation(s) for any display nodes.
* Override `void OnMRMLSceneNodeRemoved(vtkMRMLNode* node)` to watch for if a node of your type is removed from the scene. Remove corresponding widget(s) and widget representation(s).
* Override `void OnMRMLSceneEndImport()` to watch for an MRB file that has finished importing.
* Override `void OnMRMLSceneEndClose()` to clean up when a scene closes.
* Override `void ProcessMRMLNodesEvents(vtkObject *caller, unsigned long event, void *callData)` to watch for changes in the data node that would require the display to change.
* Override `void UpdateFromMRML()` and `void UpdateFromMRMLScene()` to bring the displayable manager in line with the MRML Scene.

#### The storage node

A storage node is responsible for reading and writing data nodes to files. A single data node type can have multiple storage node types associated with it for reading/writing different formats. A storage node will be created for both normal save/load operations for a single data node, as well as when you are saving a whole scene to an MRB.

It is common for a data node’s storage node to also write relevant values out of the display node (colors, opacity, etc) at the same time it writes the data.

Note that the storage node is not sufficient in itself to allow the new data node to be saved/loaded from the normal 3DSlicer save/load facilities; the reader and writer will help with that.

Files:

* \<Extension>/\<Module>/MRML/vtkMRML\<MyCustomType>StorageNode.h
* \<Extension>/\<Module>/MRML/vtkMRML\<MyCustomType>StorageNode.cxx

Key Points:

* Naming convention for class: `vtkMRML<MyCustomType>StorageNode`
  * If you have multiple storage nodes you may have other information in the name, such as the format that is written. E.g. `vtkMRMLMarkupsJSONStorageNode`.
* Inherits from `vtkMRMLStorageNode`.
* Constructing a new node is same as the data node.
* Override `bool CanReadInReferenceNode(vtkMRMLNode *refNode)` to allow a user to inquire at runtime if a particular node can be read in by this storage node.
* Override protected `void InitializeSupportedReadFileTypes()` to show what file types and extensions this storage node can read (can be more than one).
* Override protected `void InitializeSupportedWriteFileTypes()` to show what types and extensions this storage node can read (can be more than one).
  * It is recommended to be able to read and write the same file types within a single storage node.
* Override protected `int ReadDataInternal(vtkMRMLNode *refNode)`:
  * This is called by the public `ReadData` method.
  * This is where the actually reading data from a file happens.
* Override protected `int WriteDataInternal(vtkMRMLNode *refNode)`:
  * This is called by the public `WriteData` method.
  * This is where the actually writing data to a file happens.
* If your data node uses any coordinates (most nodes that get displayed should) it is recommended to be specific in your storage format whether the saved coordinates are RAS or LPS coordinates.
  * Having a way to allow the user to specify this is even better.
* If your storage node reads/writes JSON:
  * Rapidjson is already in the superbuild and is the recommended JSON parser (https://rapidjson.org/index.html).
  * It is recommended to have your extension be `.<something>.json` where the `<something>` is related to your node type.
    * E.g `.mrk.json` for Markups.
* Other methods
  * Adding a `vtkMRML<MyCustomType>Node* Create<MyCustomType>Node(const char* nodeName)` function will be convenient for implementing the writer and is also convenient for users of the storage node.

#### The reader

The recommended way to read a file into a MRML node is through the storage node. The reader, on the other hand, exists to interface with the loading facilities of 3DSlicer (drag and drop, as well as the button to load data into the scene). As such, the reader uses the storage node in its implementation.


Files:

* \<Extension>/\<Module>/qSlicer\<MyCustomType>Reader.h
* \<Extension>/\<Module>/qSlicer\<MyCustomType>Reader.cxx

Key Points:

* Naming convention for class: `qSlicer<MyCustomType>Reader`
* Inherits from `qSlicerFileReader`.
* In the class definition, the following macros should be used:
  * `Q_OBJECT`
  * `Q_DECLARE_PRIVATE`
  * `Q_DISABLE_COPY`
* As this is a Qt style object, it often follows the D-Pointer pattern (not strictly required).
  * https://wiki.qt.io/D-Pointer
* Constructing a new node:
  * Note that this is not a VTK object, it is actually a QObject, so we follow Qt guidelines.
  * Create constructor `qSlicer<MyCustomType>Reader(QObject* parent = nullptr)`.
    * This constructor, even if it is not explicitly used, allows this file to be wrapped in Python.
* Override `QString description() const` to provide a short description on the types of files read.
* Override `IOFileType fileType() const` to give a string to associate with the types of files read.
  * This string can be used in conjunction with the python method `slicer.util.loadNodeFromFile`
* Override `QStringList extensions() const` to provide the extensions that can be read.
  * Should be the same as the storage node since the reader uses the storage node.
* Override `bool load(const IOProperties& properties)`. This is the function that actually loads the node from the file into the scene.

#### The writer

The writer is the companion to the reader, so, similar to the reader, it does not implement the actual writing of files, but rather it uses the storage node. Its existence is necessary to use 3DSlicer’s built in saving facilities, such as the save button.

Files:

* \<Extension>/\<Module>/qSlicer\<MyCustomType>Writer.h
* \<Extension>/\<Module>/qSlicer\<MyCustomType>Writer.cxx

Key points:

* Naming convention for class: `qSlicer<MyCustomType>Writer`
* Inherits from `qSlicerNodeWriter`.
* See the reader for information on defining and constructing Qt style classes.
* Override `QStringList extensions(vtkObject* object) const` to provide file extensions that can be written to.
  * File extensions may be different, but don’t have to be, for different data nodes that in the same hierarchy (e.g. Markups Curve and Plane could reasonably require different file extensions, but they don’t).
* Override `bool write(const qSlicerIO::IOProperties& properties)` to do the actual writing (by way of a storage node, of course).

#### The subject hierarchy plugin

A convenient module in 3DSlicer is the Data module. It brings all the different data types together under one roof and offers operations such as cloning, deleting, and renaming nodes that work regardless of the node type. The Data module uses the Subject Hierarchy, which is what we need to plug into so our new node type can be seen in and modified by the Data module.

Files:

* \<Extension>/\<Module>/SubjectHierarchyPlugins/qSlicerSubjectHierarchy\<MyCustomType>Plugin.h
* \<Extension>/\<Module>/SubjectHierarchyPlugins/qSlicerSubjectHierarchy\<MyCustomType>Plugin.cxx

Key Points:

* Naming convention for class: `qSlicerSubjectHierarchy<MyCustomType>Plugin`
* Inherits from `qSlicerSubjectHierarchyAbstractPlugin`.
* See the reader for information on defining and constructing Qt style classes.
* Override `double canAddNodeToSubjectHierarchy(vtkMRMLNode* node, vtkIdType parentItemID=vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID) const`.
  * This method is used to determine if a data node can be placed in the hierarchy using this plugin.
* Override `double canOwnSubjectHierarchyItem(vtkIdType itemID) const` to say if this plugin can own a particular subject hierarchy item.
* Override `const QString roleForPlugin() const` to give the plugin’s role (most often meaning the data type the plugin can handle, e.g. Markup).
* Override `QIcon icon(vtkIdType itemID)` and `QIcon visibilityIcon(int visible)` to set icons for your node type.
* Override `QString tooltip(vtkIdType itemID) const` to set a tool tip for your node type.
* Override the following to determine what happens when a user gets/sets the node color through the subject hierarchy:
  * `void setDisplayColor(vtkIdType itemID, QColor color, QMap<int, QVariant> terminologyMetaData)`
  * `QColor getDisplayColor(vtkIdType itemID, QMap<int, QVariant> &terminologyMetaData) const`

#### The module (aka putting it all together)

If you have used 3DSlicer for any length of time, you have probably noticed that for each type of node (or set of types as in something like markups) there is a dedicated module that is used solely for interacting with the single node type (or set of types). Examples would be the Models, Volumes, and Markups modules. These modules are useful from a user perspective and also necessary to get your new node registered everywhere it needs to be.

As these are normal 3DSlicer modules, they come in three main parts, the module, the logic, and the module widget. The recommended way to create a new module is through the Extension Wizard (https://www.slicer.org/wiki/Documentation/Nightly/Developers/ExtensionWizard)

Files:

* \<Extension>/\<Module>/qSlicer\<MyCustomType>Module.h
* \<Extension>/\<Module>/qSlicer\<MyCustomType>Module.cxx
* \<Extension>/\<Module>/qSlicer\<MyCustomType>ModuleWidget.h
* \<Extension>/\<Module>/qSlicer\<MyCustomType>ModuleWidget.cxx
* \<Extension>/\<Module>/Logic/vtkSlicer\<MyCustomType>Logic.h
* \<Extension>/\<Module>/Logic/vtkSlicer\<MyCustomType>Logic.cxx

In qSlicer\<MyCustomType>Module.cxx:

* Override the `void setup()` function:
  * Register your displayable manager with the `vtkMRMLThreeDViewDisplayableManagerFactory` and/or the `vtkMRMLSliceViewDisplayableManagerFactory`.
  * Register your subject hierarchy plugin with the `qSlicerSubjectHierarchyPluginHandler`.
  * Register your reader and writer with the `qSlicerIOManager`.
* Override the `QStringList associatedNodeTypes() const` function and return all the new MRML classes created (data, display, and storage nodes).

In vtkSlicer\<MyCustomType>Logic.cxx:

* Override the protected `void RegisterNodes()` function and register all the new MRML classes created (data, display, and storage nodes) with the MRML scene.

In qSlicer\<MyCustomType>ModuleWidget.cxx:

* Override `bool setEditedNode(vtkMRMLNode* node, QString role = QString(), QString context = QString())` and `double nodeEditable(vtkMRMLNode* node)` if you want this module to be connected to the Data module’s “Edit properties…” option in the right click menu.

### Slice view pipeline

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/slice_view_pipeline.png)

Another view of [VTK/MRML pipeline for the 2D slice views](https://www.slicer.org/wiki/File:SliceView.pptx).

Notes: the MapToWindowLevelColors has no lookup table set, so it maps the scalar volume data to 0,255 with no "color" operation.  This is controlled by the Window/Level settings of the volume display node.  The MapToColors applies the current lookup table to go from 0-255 to full RGBA.

### Layout

A layout manager ([qSlicerLayoutManager][qSlicerLayoutManager-apidoc]) shows or hides layouts:

- It instantiates, shows or hides relevant view widgets.
- It is associated with a [vtkMRMLLayoutNode][vtkMRMLLayoutNode-apidoc] describing the current layout configuration and ensuring it can be saved and restored.
- It owns an instance of [vtkMRMLLayoutLogic][vtkMRMLLayoutLogic-apidoc] that controls the layout node and the view nodes in a MRML scene.
- Pre-defined layouts are described using XML and are registered in `vtkMRMLLayoutLogic::AddDefaultLayouts()`.
- Developer may register additional layout.

[qSlicerLayoutManager-apidoc]: http://apidocs.slicer.org/master/classqSlicerLayoutManager.html
[vtkMRMLLayoutNode-apidoc]: http://apidocs.slicer.org/master/classvtkMRMLLayoutNode.html
[vtkMRMLLayoutLogic-apidoc]: http://apidocs.slicer.org/master/classvtkMRMLLayoutLogic.html

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
