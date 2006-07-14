
catch "t Delete"
catch "f Delete"
catch "b Delete"
catch "slicerApp Delete"
catch "appLogic Delete"
catch "scene Delete"
catch "sliceGUI Delete"

source $::SLICER_BUILD/SliceViewerInteractor.tcl

vtkSlicerApplication slicerApp
slicerApp StartApplication

vtkSlicerApplicationLogic appLogic
vtkMRMLScene scene
namespace eval slicer3 set MRMLScene scene
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

  vtkKWPushButton b
  b SetParent f
  b SetText "Quit"
  set ::quit 0
  b SetCommand slicerApp "Evaluate {set ::quit 1}"
  b Create
  pack [b GetWidgetName]

  t SetSize 512 512
  t SetPosition 100 100
  # t HideDecorationOn
  t Display


  if { 1 } {

    catch "volumeNode Delete"
    catch "displayNode Delete"
    catch "storageNode Delete"

    vtkMRMLScalarVolumeNode volumeNode
    volumeNode CreateNoneNode scene
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
    #set dicomArchetype c:/tmp/S2.001
    set dicomArchetype /tmp/1.IMA
    storageNode SetFileArchetype $dicomArchetype
    storageNode ReadData volumeNode


    if { 1 } {

      catch "sliceLogic Delete"
      vtkSlicerSliceLogic sliceLogic
      sliceLogic SetMRMLScene scene
      sliceLogic ProcessMRMLEvents
      sliceLogic ProcessLogicEvents
      sliceLogic SetAndObserveMRMLScene scene

        if { 1 } {
          vtkSlicerSliceGUI sliceGUI

          if { 1 } {

            sliceGUI SetApplication slicerApp
            sliceGUI SetApplicationLogic appLogic
            sliceGUI BuildGUI f
            sliceGUI SetAndObserveMRMLScene scene
            sliceGUI SetAndObserveModuleLogic sliceLogic
            sliceGUI AddGUIObservers

            [appLogic GetSelectionNode] SetActiveVolumeID [volumeNode GetID]
            appLogic PropagateVolumeSelection

            [[sliceGUI GetSliceController] GetSliceNode] SetOrientationToCoronal

            vwait ::quit
          }

          sliceGUI Delete
        }

      sliceLogic Delete
    }

    volumeNode Delete
    displayNode Delete
    storageNode Delete

  }


  b Delete
  f Delete
  t Delete

}



appLogic Delete
scene Delete

slicerApp Exit
slicerApp Delete

#tk_messageBox -message "finished"
exit
