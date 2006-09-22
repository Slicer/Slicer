
set slicerApp [vtkSlicerApplication New]

set theme [$slicerApp GetSlicerTheme]
$slicerApp InstallTheme $theme

set scene [vtkMRMLScene New]

set appGUI [vtkSlicerApplicationGUI New]

set appLogic [vtkSlicerApplicationLogic New]
$appLogic SetMRMLScene $scene

$appGUI SetApplication $slicerApp
$appGUI SetAndObserveApplicationLogic $appLogic
$appGUI SetAndObserveMRMLScene $scene
$appGUI BuildGUI
$appGUI AddGUIObservers

set res [$slicerApp StartApplication]

$slicerApp Exit


$appGUI Delete

$slicerApp SetTheme ""
$theme SetApplication ""

$appLogic Delete

if { [$slicerApp GetBalloonHelpManager] != "" } {
  [$slicerApp GetBalloonHelpManager] SetApplication ""
}
$slicerApp Delete
$scene Delete


exit 0

