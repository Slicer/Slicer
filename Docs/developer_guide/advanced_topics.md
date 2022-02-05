# Advanced Topics

## Memory Management


### Pointers to VTK object

If you are not familiar with VTK's memory management read this [general introduction](http://aosabook.org/en/vtk.html) and this [page describing usage of smart pointers](http://www.vtk.org/Wiki/VTK/Tutorials/SmartPointers). See Slicer-specific use cases and recommendations below.

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

Factory methods return a pointer to a VTK object (with a reference count of 1) that the caller "owns", so the caller must take care of releasing the object to avoid memory leak.

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

Note: MRML scene's `CreateNodeByClass` creates a node with the default settings set in the scene for that node type (using [vtkMRMLScene::AddDefaultNode](https://www.slicer.org/doc/html/classvtkMRMLScene.html#ae302c5ed4aabb2910bc35dcc9aa2513f)).

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
