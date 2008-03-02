package provide SBuildPlugins 1.0

lappend ::SBuild(Plugins) TEEM

set ::Plugin(TEEM,Type) "required"
set ::Plugin(TEEM,Order) 23
set ::Plugin(TEEM,CanUseUserBuild) 1

# Here is where we would add lines to the main Slicer3 configuration
proc TEEM-Setup {} {
  global SBuild
}

proc TEEM-ConfigureExternal {} {
  global SBuild
  set SBuild(TEEM,LibPath) [file dirname [FindFile $::SBuild(TEEM,ExternalBuildPath) [list libTEEMCommon* TEEMCommon*.lib TEEMCommon*.dll]]]
}

proc TEEM-ConfigureSlicer {} {
  global Plugin SBuild Slicer
  set dir [file join $SBuild(SlicerLibDir) TEEM-build]
  if { $SBuild(TEEM,UseExternalBuild) } {
    set dir $SBuild(TEEM,ExternalBuildPath)
  }
  Debug "setting TEEM_DIR to $dir"
  lappend Slicer(CMakeArguments) -DTEEM_DIR:FILEPATH=$dir
}

proc TEEM-Update {} {
  Debug "Checking out TEEM"
  global SBuild Plugin
  file mkdir TEEM
  ExecuteCommand $SBuild(SVNCommand) co http://www.na-mic.org/svn/Slicer3-lib-mirrors/trunk/teem teem
}


proc TEEM-Configure { {zlib "libvtkzlib.so"} {png "libvtkpng.so"} {cflags ""}} {
  Debug "Configure TEEM"
  global SBuild Plugin
  file mkdir teem-build
  cd teem-build

  # Beware that ExecutePluginMethod resets the working directory
  set cwd [pwd]
  set vtk [ExecutePluginMethod VTK GetHome]
  if { $vtk == "" } { 
    Error "Cannot find VTK, TEEM cannot be configured!"
    tk_messageBox -message "Cannot find VTK build which is required to build TEEM" -type ok -icon error
  }
  set vtksource [ExecutePluginMethod VTK GetSourceDir]
  if { $vtksource == "" } {
    Error "Cannot find VTK Source, TEEM cannot be configured!"
    tk_messageBox -message "Cannot find VTK which is required to build TEEM" -type ok -icon error
    return
  }
  set BuildSubDir ""
  if { $SBuild(isWindows) } {
    BuildSubDir $SBuild(BuildType)
  }

  cd $cwd
  Error "Configure [pwd]"
  ExecuteCommand $SBuild(CMake) \
    -G$SBuild(Generator) \
    -DCMAKE_VERBOSE_MAKEFILE:BOOL=OFF \
    -DCMAKE_CXX_COMPILER:STRING=$::SBuild(CompilerPath)/$SBuild(Compiler) \
    -DCMAKE_CXX_COMPILER_FULLPATH:FILEPATH=$::SBuild(CompilerPath)/$SBuild(Compiler) \
    $cflags \
    -DBUILD_TESTING:BOOL=OFF \
    -DTEEM_ZLIB:BOOL=ON \
    -DTEEM_PNG:BOOL=ON \
    -DBUILD_VTK_CAPATIBLE_TEEM:BOOL=ON \
    -DCMAKE_BUILD_TYPE:STRING=$::SBuild(BuildType) \
    -DCMAKE_CXX_FLAGS_DEBUG:STRING=$::SBuild(CMakeCXXFlagsDebug) \
    -DTEEM_VTK_TOOLKITS_IPATH:FILEPATH=$vtk \
    -DZLIB_INCLUDE_DIR:PATH=[file join $vtksource Utilities] \
    -DTEEM_ZLIB_DLLCONF_IPATH:PATH=[file join $vtk Utilities] \
    -DZLIB_LIBRARY:FILEPATH=$vtk/bin/$BuildSubDir/$zlib \
    -DPNG_PNG_INCLUDE_DIR:PATH=$vtksource/Utilities/vtkpng \
    -DTEEM_PNG_DLLCONF_IPATH:PATH=$vtk/Utilities \
    -DPNG_LIBRARY:FILEPATH=$vtk/bin/$BuildSubDir/$png \
    -DTEEM_SUBLIBRARIES:BOOL=TRUE \
    ../teem
}

proc TEEM-Configure-Darwin {} {
  return [TEEM-Configure "libvtkzlib.dylib" "libvtkpng.dylib" "-DCMAKE_C_FLAGS:STRING=-fno-common"]
}

proc TEEM-Configure-Linux {} {
  return [TEEM-Configure "libvtkzlib.so" "libvtkpng.so"]
}

proc TEEM-Configure-Windows {} {
  return [TEEM-Configure "vtkzlib.lib" "vtkpng.lib"]
}

proc TEEM-Build-Windows {} {
  global SBuild
  cd TEEM-build
  ExecuteCommand $SBuild(Make) TEEM.SLN /build  $SBuild(BuildType)
}

proc TEEM-Build {} {
  Debug "Building TEEM"
  global SBuild
  cd TEEM-build
  eval ExecuteCommand $SBuild(Make) $SBuild(ParallelMake)
}

proc TEEM-Clean {} {
  Debug "Cleaning TEEM"
}



