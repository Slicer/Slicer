package provide SBuildPlugins 1.0

lappend ::SBuild(Plugins) CMake

set ::Plugin(CMake,Type) "required"
set ::Plugin(CMake,Order) 0
set ::Plugin(CMake,Directory) CMake-build
set ::Plugin(CMake,CanUseUserBuild) 1
set ::Plugin(CMake,Tag) "CMake-2-4-2"


proc CMake-Setup {} {
  # Called to initialize the plugin
  global SBuild
}

proc CMake-ConfigureExternal {} {
  global SBuild
  set SBuild(CMake) [FindFile $::SBuild(CMake,ExternalBuildPath) [list cmake cmake.exe]]
}
  

# Here is where we would add lines to the main Slicer3 configuration
proc CMake-ConfigureSlicer {} {
  global Plugin SBuild
  CMake-Setup
}

proc CMake-Update-Windows {} {
  Debug "Checking out CMake"
  global SBuild Plugin
  ExecuteCommand $SBuild(SVNCommand) co http://www.na-mic.org/svn/Slicer3-lib-mirrors/trunk/Binaries/Windows/CMake-build CMake-build
}

proc CMake-Update {} {
  Debug "Checking out CMake"
  global SBuild Plugin
  ExecuteCommand $SBuild(SVNCommand) co http://www.na-mic.org/svn/Slicer3-lib-mirrors/trunk/CMake CMake
}

proc CMake-Configure-Windows {} {
  Log "Nothing to be done to configure CMake for Windows"
}

proc CMake-Configure {} {
  Log "Configure Windows"
  global SBuild Plugin
  file mkdir $Plugin(CMake,Directory)
  cd $Plugin(CMake,Directory)
  ExecuteCommand $SBuild(SlicerLibDir)/CMake/bootstrap
}

proc CMake-Build {} {
  Info "Building CMake"
  global SBuild Plugin
  cd $Plugin(CMake,Directory)
  eval ExecuteCommand $SBuild(Make)
  set SBuild(CMake) $SBuild(SlicerLibDir)/CMake-build/bin/cmake
  Info "CMake build complete"
  SavePreferences
}

proc CMake-Build-Windows {} {
  global SBuild
  Debug "Building CMake - nothing for Windows"
  set SBuild(CMake) $SBuild(SlicerLibDir)/CMake-buid/bin/cmake.exe
  Info "CMake build complete"
  SavePreferences
}

proc CMake-Clean {} {
  Debug "Cleaning CMake"
  cd $Plugin(CMake,Directory)
  eval ExecuteCommand $SBuild(Make) clean
}

proc CMake-Clean-Windows {} {
  Debug "Cleaning CMake"
}

