

#
# utilities for testing event broker
#

proc EventBrokerTmpDir {} {
  if { [file isdirectory g:/tmp/broker] } {
    return g:/tmp/broker
  }
  return $::env(SLICER_HOME)/Testing/Temporary
}

proc EventBrokerPrint {} {
  puts [$::slicer3::Broker Print]
}

proc EventBrokerGraph { {fileName broker.dot} } {
  $::slicer3::Broker GenerateGraphFile [EventBrokerTmpDir]/$fileName
}

proc EventBrokerUpdate {} {
  $::slicer3::Broker ProcessEventQueue
}

proc EventBrokerAsync { {fileName broker.log} } {

  $::slicer3::Broker SetLogFileName [EventBrokerTmpDir]/$fileName
  $::slicer3::Broker EventLoggingOn
  $::slicer3::Broker SetEventModeToAsynchronous
}

proc EventBrokerLogCommand { cmd {fileName brokercmd.log} } {

  $::slicer3::Broker SetLogFileName [EventBrokerTmpDir]/$fileName
  $::slicer3::Broker EventLoggingOn
  $::slicer3::Broker OpenLogFile

  eval $cmd
  set timer [time "eval $cmd"]

  puts $timer

  $::slicer3::Broker CloseLogFile
  $::slicer3::Broker EventLoggingOff
}

proc EventBrokerLoadSampleScene { {sceneFileName ""} } {
  if { $sceneFileName == "" } {
    set sceneFileName $::env(SLICER_HOME)/share/MRML/Testing/vol_and_cube_camera.mrml
  }

  $::slicer3::MRMLScene SetURL $sceneFileName
  $::slicer3::MRMLScene Connect
  update

}

proc EventBrokerEnterVolumes {} {
  set toolbar [$::slicer3::ApplicationGUI GetApplicationToolbar]
  [$toolbar GetModuleChooseGUI] SelectModule "Volumes"
}

proc EventBrokerTests {} {

  EventBrokerLogCommand "$::slicer3::MRMLScene Modified" scenemod.dot
  EventBrokerLogCommand "EventBrokerLoadSampleScene" sceneload.dot
  EventBrokerLogCommand "EventBrokerEnterVolumes" entervolumes.dot
  puts $::slicer3::Broker
}
