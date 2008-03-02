package provide SBuildPlugins 1.0

lappend ::SBuild(Plugins) KWWidgets

set ::Plugin(KWWidgets,Type) "required"
set ::Plugin(KWWidgets,Order) 30
set ::Plugin(KWWidgets,CanUseUserBuild) 1

# Here is where we would add lines to the main Slicer3 configuration
proc KWWidgets-Setup {} {
  global SBuild
  Debug "configuring KWWidgets"
}

proc KWWidgets-ConfigureExternal {} {
  global SBuild
  set SBuild(KWWidgets,LibPath) [file dirname [FindFile $::SBuild(KWWidgets,ExternalBuildPath) [list KWWidgetsConfig.cmake]]]
}

proc KWWidgets-ConfigureSlicer {} {
  global Plugin SBuild Slicer
  set dir [file join $SBuild(SlicerLibDir) KWWidgets-build]
  if { $SBuild(KWWidgets,UseExternalBuild) } {
    set dir $SBuild(KWWidgets,ExternalBuildPath)
  }
  Debug "setting KWWidgets_DIR to $dir"
  lappend Slicer(CMakeArguments) -DKWWidgets_DIR:FILEPATH=$dir
}

proc KWWidgets-Update {} {
  Debug "Checking out KWWidgets"
  global SBuild Plugin
  file mkdir KWWidgets
  ExecuteCommand $SBuild(SVNCommand) co http://www.na-mic.org/svn/Slicer3-lib-mirrors/trunk/KWWidgets KWWidgets
}

proc KWWidgets-Configure {} {
  Debug "Configure KWWidgets"
  global SBuild Plugin
  file mkdir KWWidgets-build
  cd KWWidgets-build

  # Try to find VTK
  if { $SBuild(VTK,UseExternalBuild) } { 
    set vtk [file dirname [FindFile $::SBuild(VTK,ExternalBuildPath) [list VTKConfig.cmake]]]
  } else {
    set vtk [file dirname [FindFile [file join $::SBuild(SlicerLibDir) VTK-build] [list VTKConfig.cmake]]]
  }
  if { $vtk == "" } { 
    Error "Cannot find VTK, KWWidgets cannot be configured!"
  }
  if { $SBuild(Architecture) == "Darwin" } {
  ExecuteCommand $SBuild(CMake) \
    -G$SBuild(Generator) \
    -DCMAKE_CXX_COMPILER:STRING=$::SBuild(CompilerPath)/$SBuild(Compiler) \
    -DCMAKE_CXX_COMPILER_FULLPATH:FILEPATH=$::SBuild(CompilerPath)/$SBuild(Compiler) \
    -DBUILD_SHARED_LIBS:BOOL=ON \
    -DCMAKE_SKIP_RPATH:BOOL=ON \
    -DBUILD_EXAMPLES:BOOL=OFF \
    -DBUILD_TESTING:BOOL=OFF \
    -DCMAKE_BUILD_TYPE:STRING=$::SBuild(BuildType) \
    -DCMAKE_CXX_FLAGS_DEBUG:STRING=$::SBuild(CMakeCXXFlagsDebug) \
    -DVTK_DIR:PATH=$vtk \
    {-DCMAKE_SHARED_LINKER_FLAGS:STRING=-dylib_file /System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib } \
    {-DCMAKE_EXE_LINKER_FLAGS:STRING=-dylib_file /System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib } \
    ../KWWidgets
  } else {
  ExecuteCommand $SBuild(CMake) \
    -G$SBuild(Generator) \
    -DCMAKE_CXX_COMPILER:STRING=$::SBuild(CompilerPath)/$SBuild(Compiler) \
    -DCMAKE_CXX_COMPILER_FULLPATH:FILEPATH=$::SBuild(CompilerPath)/$SBuild(Compiler) \
    -DBUILD_SHARED_LIBS:BOOL=ON \
    -DCMAKE_SKIP_RPATH:BOOL=ON \
    -DBUILD_EXAMPLES:BOOL=OFF \
    -DBUILD_TESTING:BOOL=OFF \
    -DCMAKE_BUILD_TYPE:STRING=$::SBuild(BuildType) \
    -DCMAKE_CXX_FLAGS_DEBUG:STRING=$::SBuild(CMakeCXXFlagsDebug) \
    -DVTK_DIR:PATH=$vtk \
    ../KWWidgets
  }
}


proc KWWidgets-Build-Windows {} {
  global SBuild
  cd KWWidgets-build
  ExecuteCommand $SBuild(Make) KWWidgets.SLN /build  $SBuild(BuildType)
}

proc KWWidgets-Build {} {
  Debug "Building KWWidgets"
  global SBuild
  cd KWWidgets-build
  eval ExecuteCommand $SBuild(Make) $SBuild(ParallelMake)
}

proc KWWidgets-Clean {} {
  Debug "Cleaning KWWidgets"
}



