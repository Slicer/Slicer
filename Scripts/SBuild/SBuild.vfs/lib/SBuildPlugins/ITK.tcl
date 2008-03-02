package provide SBuildPlugins 1.0

lappend ::SBuild(Plugins) ITK

set ::Plugin(ITK,Type) "required"
set ::Plugin(ITK,Order) 22
set ::Plugin(ITK,CanUseUserBuild) 1

# Here is where we would add lines to the main Slicer3 configuration
proc ITK-Setup {} {
  global SBuild
}

proc ITK-ConfigureExternal {} {
  global SBuild
  set SBuild(ITK,LibPath) [file dirname [FindFile $::SBuild(ITK,ExternalBuildPath) [list libITKCommon* ITKCommon*.lib ITKCommon*.dll]]]
}

proc ITK-ConfigureSlicer {} {
  global Plugin SBuild Slicer
  set dir [file join $SBuild(SlicerLibDir) Insight-build]
  if { $SBuild(ITK,UseExternalBuild) } {
    set dir $SBuild(ITK,ExternalBuildPath)
  }
  Debug "setting ITK_DIR to $dir"
  lappend Slicer(CMakeArguments) -DITK_DIR:FILEPATH=$dir
}

proc ITK-Update {} {
  Debug "Checking out ITK"
  global SBuild Plugin
  file mkdir Insight
  ExecuteCommand $SBuild(SVNCommand) co http://www.na-mic.org/svn/Slicer3-lib-mirrors/trunk/Insight Insight
}

proc ITK-Configure {} {
  Debug "Configure Insight"
  global SBuild Plugin
  file mkdir Insight-build
  cd Insight-build
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
    ../Insight
}

proc ITK-Configure-Darwin {} {
  Debug "Configure Insight"
  global SBuild Plugin
  file mkdir Insight-build
  cd Insight-build
  ExecuteCommand $SBuild(CMake) \
    -G$SBuild(Generator) \
    -DCMAKE_CXX_COMPILER:STRING=$::SBuild(CompilerPath)/$SBuild(Compiler) \
    -DCMAKE_CXX_COMPILER_FULLPATH:FILEPATH=$::SBuild(CompilerPath)/$SBuild(Compiler) \
    -DBUILD_SHARED_LIBS:BOOL=ON \
    -DCMAKE_SKIP_RPATH:BOOL=OFF \
    -DBUILD_EXAMPLES:BOOL=OFF \
    -DBUILD_TESTING:BOOL=OFF \
    -DCMAKE_BUILD_TYPE:STRING=$::SBuild(BuildType) \
    -DCMAKE_CXX_FLAGS_DEBUG:STRING=$::SBuild(CMakeCXXFlagsDebug) \
    ../Insight
}

proc ITK-Build-Windows {} {
  global SBuild
  cd Insight-build
  ExecuteCommand $SBuild(Make) ITK.SLN /build  $SBuild(BuildType)
}

proc ITK-Build {} {
  Debug "Building ITK"
  global SBuild
  cd Insight-build
  eval ExecuteCommand $SBuild(Make) $SBuild(ParallelMake)
}

proc ITK-Clean {} {
  Debug "Cleaning ITK"
}



