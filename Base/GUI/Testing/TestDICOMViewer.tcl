
catch "t Delete"
catch "f Delete"
catch "slicerApp Delete"
catch "appLogic Delete"
catch "scene Delete"
catch "sliceGUI Delete"

source $::SLICER_BUILD/SliceViewerInteractor.tcl

vtkSlicerApplication slicerApp
slicerApp StartApplication

vtkSlicerApplicationLogic appLogic
vtkMRMLScene scene
appLogic SetAndObserveMRMLScene scene
appLogic ProcessMRMLEvents

if { 1 } {

  vtkKWTopLevel t
  t SetApplication slicerApp
  t Create

  vtkKWFrame f
  f SetParent t
  f Create
  pack [f GetWidgetName] -fill both -expand true

  t SetSize 512 512
  t HideDecorationOn
  t Display


  if { 1 } {

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


    #set dicomArchetype [tk_getOpenFile]
    set dicomArchetype c:/tmp/S2.001
    storageNode SetFileArchetype $dicomArchetype
    storageNode ReadData volumeNode

    [appLogic GetSelectionNode] SetActiveVolumeID [volumeNode GetID]
    appLogic PropagateVolumeSelection

    if { 1 } {

      catch "sliceLogic Delete"
      vtkSlicerSliceLogic sliceLogic
      sliceLogic SetMRMLScene scene
      sliceLogic ProcessMRMLEvents
      sliceLogic ProcessLogicEvents
      sliceLogic SetAndObserveMRMLScene scene

        if { 0 } {
          vtkSlicerSliceGUI sliceGUI
          sliceGUI SetApplication slicerApp
          sliceGUI SetApplicationLogic appLogic
          sliceGUI SetAndObserveMRMLScene scene
          sliceGUI SetAndObserveModuleLogic sliceLogic

          sliceGUI BuildGUI f
          sliceGUI AddGUIObservers

          [[sliceGUI GetSliceController] GetSliceNode] SetOrientationToCoronal
          pack forget [[sliceGUI GetSliceController] GetWidgetName]

        
          sliceGUI Delete
        }

      sliceLogic Delete
    }

    volumeNode Delete
    displayNode Delete
    storageNode Delete

  }


  t Delete
  f Delete

}


  update
  tk_messageBox -message "look!"

appLogic Delete
scene Delete

slicerApp Exit
slicerApp Delete

#tk_messageBox -message "finished"
exit
