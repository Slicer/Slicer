package provide SBuildPlugins 1.0

lappend ::SBuild(Plugins) Tk

set ::Plugin(Tk,Type) "required"
set ::Plugin(Tk,Order) 11
set ::Plugin(Tk,CanUseUserBuild) 1

# Here is where we would add lines to the main Slicer3 configuration
proc Tk-Setup {} {
  global SBuild
}

proc Tk-ConfigureExternal {} {
  global SBuild
  set SBuild(Tk,LibPath) [file dirname [FindFile $::SBuild(Tk,ExternalBuildPath) [list libtk* tk*.lib tk*.dll]]]
  set SBuild(Tk,Lib) [FindFile $::SBuild(Tcl,ExternalBuildPath) [list libtk* tk*.lib tk*.dll]]
}

proc Tk-ConfigureSlicer {} {
  global Plugin SBuild
  Debug "Setting up Tk"
}

proc Tk-Update {} {
  Debug "Checking out Tk"
  global SBuild Plugin
  file mkdir tcl
  cd tcl
  ExecuteCommand $SBuild(SVNCommand) co http://www.na-mic.org/svn/Slicer3-lib-mirrors/trunk/tcl/tk tk
}

proc Tk-Configure-Darwin {} {
  Debug "Configure Tk"
  global SBuild Plugin
  cd tcl/tk/unix
  ExecuteCommand ./configure --with-tcl=$SBuild(Tcl,LibPath) --prefix=$SBuild(SlicerLibDir)/tcl-build --disable-corefoundation
}

proc Tk-Configure {} {
  Debug "Configure Tk"
  global SBuild Plugin
  cd tcl/tk/unix
  ExecuteCommand ./configure --with-tcl=$SBuild(Tcl,LibPath) --prefix=$SBuild(SlicerLibDir)/tcl-build
}

proc Tk-Build {} {
  Debug "Building Tk"
  global SBuild
  cd tcl/tk/unix
  eval ExecuteCommand $SBuild(Make)
  eval ExecuteCommand $SBuild(Make) install
  set SBuild(Tk,LibPath) $SBuild(SlicerLibDir)/tcl-build/lib
  file copy -force $SBuild(SlicerLibDir)/tcl/tk/generic/default.h $SBuild(SlicerLibDir)/tcl-build/include
  file copy -force $SBuild(SlicerLibDir)/tcl/tk/unix/tkUnixDefault.h $SBuild(SlicerLibDir)/tcl-build/include
}

proc Tk-Clean {} {
  Debug "Cleaning Tk"
}



# Windows: nothing to be done, if Tcl is ok
proc Tk-Update-Windows {} {}
proc Tk-Configure-Windows {} {}
proc Tk-Build-Windows {} {}
