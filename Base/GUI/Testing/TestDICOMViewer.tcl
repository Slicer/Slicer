
catch "t Delete"
catch "f Delete"
catch "app Delete"
catch "appLogic Delete"
catch "scene Delete"
catch "sliceLogic Delete"
catch "sliceGUI Delete"

source $::SLICER_BUILD/SliceViewerInteractor.tcl

vtkSlicerApplication app
vtkSlicerApplicationLogic appLogic
vtkMRMLScene scene
appLogic SetAndObserveMRMLScene scene
appLogic ProcessMRMLEvents

vtkKWTopLevel t
t SetApplication app
t Create

vtkKWFrame f
f SetParent t
f Create
pack [f GetWidgetName] -fill both -expand true

vtkSlicerSliceLogic sliceLogic
sliceLogic SetMRMLScene scene
sliceLogic ProcessMRMLEvents
sliceLogic ProcessLogicEvents
sliceLogic SetAndObserveMRMLScene scene

vtkSlicerSliceGUI sliceGUI
sliceGUI SetApplication app
sliceGUI SetApplicationLogic appLogic
sliceGUI SetAndObserveMRMLScene scene
sliceGUI SetAndObserveModuleLogic sliceLogic
sliceGUI BuildGUI f
sliceGUI AddGUIObservers

t HideDecorationOn
t Display



catch "volumeNode Delete"
catch "displayNode Delete"
catch "storageNode Delete"

vtkMRMLScalarVolumeNode volumeNode
vtkMRMLVolumeDisplayNode displayNode
vtkMRMLVolumeArchetypeStorageNode storageNode

volumeNode SetScene scene
displayNode SetScene scene
storageNode SetScene scene

scene AddNode volumeNode
scene AddNode displayNode
scene AddNode storageNode

volumeNode SetName dicom
volumeNode SetStorageNodeID [storageNode GetID]
volumeNode SetDisplayNodeID [displayNode GetID]


set dicomArchetype [tk_getOpenFile]
storageNode SetFileArchetype $dicomArchetype
storageNode ReadData volumeNode


[appLogic GetSelectionNode] SetActiveVolumeID [volumeNode GetID]
appLogic PropagateVolumeSelection

[[sliceGUI GetSliceController] GetSliceNode] SetOrientationToCoronal
pack forget [[sliceGUI GetSliceController] GetWidgetName]

set quit 0
vwait quit
