#
# ExtractSubvolumeROI logic procs
#

#
# make a clone of the scripted module example with a new name
#
proc ExtractSubvolumeROIClone { moduleName {slicerSourceDir ""} {targetDir ""} } {

  if { $slicerSourceDir == "" } {
    set slicerSourceDir $::env(Slicer3_HOME)/../Slicer3
  }
  if { $targetDir == "" } {
    set targetDir $slicerSourceDir/Modules/$moduleName
  }
  if { [file exists $targetDir] } {
    error "Target dir exists - please delete $targetDir"
  }
  puts "Making $targetDir"
  file mkdir $targetDir

  set files [glob $slicerSourceDir/Modules/ExtractSubvolumeROI/*]
  foreach f $files {
    set ff [file tail $f]
    if { [string match ".*" $ff] || [string match "*~" $ff] } {
      continue
    }
    set fp [open $f "r"]
    set contents [read $fp]
    close $fp
    regsub -all "ExtractSubvolumeROI" $contents $moduleName newContents

    regsub -all "ExtractSubvolumeROI" $ff $moduleName newFileName
    set fp [open $targetDir/$newFileName "w"]
    puts -nonewline $fp $newContents
    close $fp

    puts "  created file $newFileName"
  }
  puts "New module created - add entry to $slicerSourceDir/Modules/CMakeLists.txt"
}

proc ExtractSubvolumeROICreateLabelVolume {this} {
  set volumeNode $::ExtractSubvolumeROI($this,inputVolume)
  set scene [[$this GetLogic] GetMRMLScene]

  set volumesLogic [$::slicer3::VolumesGUI GetLogic]
  set labelVolumeName [[$::ExtractSubvolumeROI($this,outputLabelText) GetWidget] GetValue]
  set labelNode [$volumesLogic CreateLabelVolume $scene $volumeNode $labelVolumeName]

  # make the source node the active background, and the label node the active label
  set selectionNode [[[$this GetLogic] GetApplicationLogic]  GetSelectionNode]
  $selectionNode SetReferenceActiveVolumeID [$volumeNode GetID]
  $selectionNode SetReferenceActiveLabelVolumeID [$labelNode GetID]
  $selectionNode Modified
  [[$this GetLogic] GetApplicationLogic]  PropagateVolumeSelection

  set ::ExtractSubvolumeROI($this,labelVolume) $labelNode
  
  set labelVolumeName [$labelNode GetName]
  $::ExtractSubvolumeROI($this,currentOutputText) SetText "Current ouput volume: $labelVolumeName"

  # update the editor range to be the full range of the background image
  set range [[$volumeNode GetImageData] GetScalarRange]
  eval ::Labler::SetPaintRange $range
}

