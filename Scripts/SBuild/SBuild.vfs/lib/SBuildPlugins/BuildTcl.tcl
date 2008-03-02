package provide SBuildPlugins 1.0

lappend ::SBuild(Plugins) Tcl

set ::Plugin(Tcl,Type) "required"
set ::Plugin(Tcl,Order) 0

# Here is where we would add lines to the main Slicer3 configuration
proc Tcl-ConfigureSlicer {} {
  global Plugin SBuild
  Debug "Setting up Tcl"
}

proc Tcl-Update {} {
  Debug "Checking out Tcl"
}

proc Tcl-Configure {} {
  Debug "Configure Tcl"
}

proc Tcl-Build {} {
  Debug "Building Tcl"
}

proc Tcl-Clean {} {
  Debug "Cleaning Tcl"
}

