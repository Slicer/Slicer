#
# CropVolume logic procs
#

#
# make a clone of the scripted module example with a new name
#
proc CropVolumeClone { moduleName {slicerSourceDir ""} {targetDir ""} } {

  if { $slicerSourceDir == "" } {
    set slicerSourceDir $::env(Slicer_HOME)/../Slicer3
  }
  if { $targetDir == "" } {
    set targetDir $slicerSourceDir/Modules/$moduleName
  }
  if { [file exists $targetDir] } {
    error "Target dir exists - please delete $targetDir"
  }
  puts "Making $targetDir"
  file mkdir $targetDir

  set files [glob $slicerSourceDir/Modules/CropVolume/*]
  foreach f $files {
    set ff [file tail $f]
    if { [string match ".*" $ff] || [string match "*~" $ff] } {
      continue
    }
    set fp [open $f "r"]
    set contents [read $fp]
    close $fp
    regsub -all "CropVolume" $contents $moduleName newContents

    regsub -all "CropVolume" $ff $moduleName newFileName
    set fp [open $targetDir/$newFileName "w"]
    puts -nonewline $fp $newContents
    close $fp

    puts "  created file $newFileName"
  }
  puts "New module created - add entry to $slicerSourceDir/Modules/CMakeLists.txt"
}
