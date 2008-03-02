package provide SBuildPlugins 1.0

lappend ::SBuild(Plugins) Tcl

set ::Plugin(Tcl,Type) "required"
set ::Plugin(Tcl,Order) 10
set ::Plugin(Tcl,CanUseUserBuild) 1

# Here is where we would add lines to the main Slicer3 configuration
proc Tcl-Setup {} {
  global SBuild
}

proc Tcl-ConfigureExternal {} {
  global SBuild
  set SBuild(Tcl,LibPath) [file dirname [FindFile $::SBuild(Tcl,ExternalBuildPath) [list libtcl* tcl*.lib tcl*.dll]]]
  set SBuild(Tcl,IncludePath) [file dirname [FindFile $::SBuild(Tcl,ExternalBuildPath) tcl.h]]
  set SBuild(Tcl,Lib) [FindFile $::SBuild(Tcl,ExternalBuildPath) [list libtcl* tcl*.lib tcl*.dll]]
  set SBuild(Tcl,tclsh) [FindFile $::SBuild(Tcl,ExternalBuildPath) tclsh*]

}

proc Tcl-ConfigureSlicer {} {
  global Plugin SBuild
  Debug "Setting up Tcl"
}

proc Tcl-Update {} {
  Debug "Checking out Tcl"
  global SBuild Plugin
  file mkdir tcl
  cd tcl
  ExecuteCommand $SBuild(SVNCommand) co http://www.na-mic.org/svn/Slicer3-lib-mirrors/trunk/tcl/tcl tcl
}

proc Tcl-Configure-Darwin {} {
  Debug "Configure Tcl"
  global SBuild Plugin
  cd tcl/tcl/unix
  ExecuteCommand ./configure --prefix=$SBuild(SlicerLibDir)/tcl-build --disable-corefoundation
}

proc Tcl-Configure {} {
  Debug "Configure Tcl"
  global SBuild Plugin
  cd tcl/tcl/unix
  ExecuteCommand ./configure --prefix=$SBuild(SlicerLibDir)/tcl-build
}

proc Tcl-Build {} {
  Debug "Building Tcl"
  global SBuild
  cd tcl/tcl/unix
  eval ExecuteCommand $SBuild(Make)
  eval ExecuteCommand $SBuild(Make) install
  set SBuild(Tcl,LibPath) $SBuild(SlicerLibDir)/tcl-build/lib
}

proc Tcl-Clean {} {
  Debug "Cleaning Tcl"
}



# Windows functions
proc Tcl-Update-Windows {} {
  Debug "Checking out Tcl"
  ExecuteCommand $SBuild(SVNCommand) co http://www.na-mic.org/svn/Slicer3-lib-mirrors/trunk/Binaries/Windows/tcl-build tcl-build
  Tcl-Update
}

# Do nothing
proc Tcl-Configure-Windows {} {}
proc Tcl-Build-Windows {} {}
