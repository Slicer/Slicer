
proc TestModuleDependency {} {

  set ::gui [$::slicer3::Application GetModuleGUIByName "OpenIGTLink IF"] 
  if {$::gui == ""} {
      puts "Cannot find OpenIGTLinkIF module.\n"
      return 1
  }

  return 0;
}

set ret [ catch { $::slicer3::Application TraceScript TestModuleDependency } ]

if {$ret == 1} {
    exit 1
} else {
    exit 0
}

