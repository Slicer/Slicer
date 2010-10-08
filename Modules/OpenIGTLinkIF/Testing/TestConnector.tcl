
proc SetupConnector {} {

  $::slicer3::MRMLScene Clear 0

  #set ::gui [$::slicer3::Application GetModuleGUIByName OpenIGTLinkIF] 
  #if {$::gui == ""} {
  #    puts "Cannot find OpenIGTLinkIF module"
  #    return 1
  #}
  #
  ##$::seed OverwritePolyDataWarningOff
  #set ::logic [$::gui GetLogic]

  #set testData $::env(Slicer_HOME)/share/Slicer/Modules/OpenIGTLinkIF/Testing/TestData/twoFibers.vtk
  #set testData c:/data/tracts/Seeding1.vtp

  set conNode [vtkMRMLIGTLConnectorNode New]
  $::slicer3::MRMLScene AddNode $conNode

  $conNode SetRestrictDeviceName 1
  $conNode SetCheckCRC 1
  $conNode SetTypeClient "127.0.0.1" 18944
  $conNode Modified
  #$conNode Start
  #$conNode Stop
  
  $conNode Delete

  return 0
}

update

set retval [SetupConnector]

if {$retval == 1} {
    puts "Error setting up OpenIGTLink"
    exit 1
} else {
    #$::slicer3::Application TraceScript "TestColor Rainbow"
    #$::slicer3::Application TraceScript "TestColor Ocean"
    exit 0
}

