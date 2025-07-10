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
