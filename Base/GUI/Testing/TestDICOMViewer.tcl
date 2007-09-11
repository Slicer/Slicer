
if { [file exists c:/Tcl/bin/tkcon.tcl] } {
  source c:/Tcl/bin/tkcon.tcl
  tkcon::Init
  tkcon::Attach Main
}

set ::vtkObjects ""

proc vtkNew {class} {
  set o [$class New]
  set ::vtkObjects "$o $::vtkObjects"
  return $o
}

proc vtkDelete {} {
  foreach o $::vtkObjects {
    $o Delete
  }
  set ::vtkObjects ""
}

##

set slicerApp [vtkNew vtkSlicerApplication]
$slicerApp StartApplication

set appLogic [vtkNew vtkSlicerApplicationLogic]
set scene [vtkNew vtkMRMLScene]
namespace eval slicer3 set MRMLScene $scene
$appLogic SetAndObserveMRMLScene $scene
$appLogic ProcessMRMLEvents

set colorLogic [vtkSlicerColorLogic New]
set colorEvents [vtkIntArray New]
$colorLogic SetAndObserveMRMLSceneEvents $scene  $colorEvents
$colorEvents Delete
$colorLogic AddDefaultColorNodes
     
if { 1 } {

  set topLevel [vtkNew vtkKWTopLevel]
  $topLevel SetApplication $slicerApp
  $topLevel Create

  set frame [vtkNew vtkKWFrame]
  $frame SetParent $topLevel
  $frame Create
  pack [$frame GetWidgetName] -fill both -expand true

  set pushButton [vtkNew vtkKWPushButton]
  $pushButton SetParent $frame
  $pushButton SetText "Quit"
  set ::quit 0
  $pushButton SetCommand $slicerApp "Evaluate {set ::quit 1}"
  $pushButton Create
  pack [$pushButton GetWidgetName]

  $topLevel SetSize 512 512
  $topLevel SetPosition 100 100
  # $topLevel HideDecorationOn
  $topLevel Display


  if { 1 } {

    set volumeNode [vtkNew vtkMRMLScalarVolumeNode]
    set displayNode [vtkNew vtkMRMLScalarVolumeDisplayNode]
    set storageNode [vtkNew vtkMRMLVolumeArchetypeStorageNode]

    $volumeNode SetScene $scene
    $displayNode SetScene $scene
    $displayNode SetAndObserveColorNodeID "vtkMRMLColorNodeGrey"
    $storageNode SetScene $scene

    $scene AddNode $volumeNode
    $scene AddNode $displayNode
    $scene AddNode $storageNode

    $volumeNode SetName dicom
    $volumeNode SetStorageNodeID [$storageNode GetID]
    $volumeNode SetAndObserveDisplayNodeID [$displayNode GetID]


    if { 0 && [file exists c:/tmp/S2.001] } {
      set dicomArchetype c:/tmp/S2.001
    }
    if { 0 && [file exists /tmp/1.IMA] } {
      set dicomArchetype /tmp/1.IMA
    }
    if { ![info exists dicomArchetype] } {
      set dicomArchetype [tk_getOpenFile]
    }

    $storageNode SetFileName $dicomArchetype
    $storageNode ReadData $volumeNode


    if { 1 } {

      catch "sliceLogic Delete"
      set sliceLogic [vtkNew vtkSlicerSliceLogic]
      $sliceLogic SetName "DICOMViewer"
      $sliceLogic SetMRMLScene $scene
      $sliceLogic ProcessMRMLEvents
      $sliceLogic ProcessLogicEvents
      $sliceLogic SetAndObserveMRMLScene $scene

        if { 1 } {
          set sliceGUI [vtkNew vtkSlicerSliceGUI]

          if { 1 } {

            $sliceGUI SetApplication $slicerApp
            $sliceGUI SetApplicationLogic $appLogic
            $sliceGUI BuildGUI $frame
            $sliceGUI SetAndObserveMRMLScene $scene
            $sliceGUI SetAndObserveModuleLogic $sliceLogic
            $sliceGUI AddGUIObservers

            [$appLogic GetSelectionNode] SetActiveVolumeID [$volumeNode GetID]
            $appLogic PropagateVolumeSelection

            [[$sliceGUI GetSliceController] GetSliceNode] SetOrientationToCoronal

          }
        }
    }
  }
}

vwait ::quit

$slicerApp Exit

vtkDelete

#tk_messageBox -message "finished"
exit
