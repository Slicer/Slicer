vtkSlicerApplication app
vtkMRMLScene scene

vtkKWTopLevel t
t SetApplication app
t Create

vtkSlicerDiffusionEditorWidget editor
editor SetParent t
editor SetAndObserveMRMLScene scene
editor Create

#set sceneFileName $::env(SLICER_HOME)/../Slicer3/Modules/Volumes/Testing/diffusionScene.mrml
 
#scene SetURL $sceneFileName
#scene Connect
#update

#editor UpdateWidget [scene GetNthNodeByClass 0 vtkMRMLDiffusionWeightedVolumeNode]

#set fiducialSelector [testWidget GetFiducialSelector] 

#fiducialSelector SetSelected [scene GetNextNodeByClass "vtkMRMLFiducialListNode"]

#set runButton [testWidget GetRunButton]

#[editor  GetTestingWidget] ProcessWidgetEvents runButton vtkKWPushButton::InvokedEvent 0

t Delete
editor Delete
scene Delete
app Delete

exit 0
