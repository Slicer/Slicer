package provide SBuildPlugins 1.0

lappend ::SBuild(Plugins) VTK

set ::Plugin(VTK,Type) "required"
set ::Plugin(VTK,Order) 20
set ::Plugin(VTK,CanUseUserBuild) 1

# Here is where we would add lines to the main Slicer3 configuration
proc VTK-Setup {} {
  global SBuild
}

proc VTK-GetHome {} {
  global SBuild
  set vtk ""
  if { $SBuild(VTK,UseExternalBuild) } { 
    set vtk [file dirname [FindFile $::SBuild(VTK,ExternalBuildPath) [list VTKConfig.cmake]]]
  } else {
    set vtk [file dirname [FindFile [file join $::SBuild(SlicerLibDir) VTK-build] [list VTKConfig.cmake]]]
  }
  return $vtk
}

proc VTK-GetSourceDir {} {
  global SBuild
  Debug "VTK-GetSourceDir: UseExternalBuild == $SBuild(VTK,UseExternalBuild)"
  if { $SBuild(VTK,UseExternalBuild) } {
    # No good way to find the source...
    
    return ""
  } else {
    Debug "VTK-GetSourceDir - Returning: [file join $SBuild(SlicerLibDir) VTK]"
    return [file join $SBuild(SlicerLibDir) VTK]
  }
}

proc VTK-ConfigureExternal {} {
  global SBuild
  set SBuild(VTK,LibPath) [file dirname [FindFile $::SBuild(VTK,ExternalBuildPath) [list libvtkCommon* vtkCommon*.lib vtkCommon*.dll]]]
}

proc VTK-ConfigureSlicer {} {
  global Plugin SBuild
}

proc VTK-Update {} {
  Debug "Checking out VTK"
  global SBuild Plugin
  file mkdir VTK
  ExecuteCommand $SBuild(SVNCommand) co http://www.na-mic.org/svn/Slicer3-lib-mirrors/trunk/VTK VTK
}

proc VTK-Configure-Windows {} {
  Debug "Configure VTK"
  global SBuild Plugin
  cd VTK
  ExecuteCommand $SBuild(CMake) \
    -G$SBuild(Generator) \
    -DCMAKE_BUILD_TYPE:STRING=$::SBuild(BuildType) \
    -DCMAKE_CXX_FLAGS_DEBUG:STRING=$::SBuild(CMakeCXXFlagsDebug) \
    -DBUILD_SHARED_LIBS:BOOL=ON \
    -DCMAKE_SKIP_RPATH:BOOL=ON \
    -DCMAKE_CXX_COMPILER:STRING=$::SBuild(CompilerPath)/$SBuild(Compiler) \
    -DCMAKE_CXX_COMPILER_FULLPATH:FILEPATH=$::SBuild(CompilerPath)/$SBuild(Compiler) \
    -DBUILD_TESTING:BOOL=OFF \
    -DVTK_USE_CARBON:BOOL=OFF \
    -DVTK_USE_X:BOOL=ON \
    -DVTK_WRAP_TCL:BOOL=ON \
    -DVTK_USE_HYBRID:BOOL=ON \
    -DVTK_USE_PATENTED:BOOL=ON \
    -DTCL_INCLUDE_PATH:PATH=$SBuild(Tcl,IncludePath) \
    -DTK_INCLUDE_PATH:PATH=$SBuild(Tcl,IncludePath) \
    -DTCL_LIBRARY:FILEPATH=$::SBuild(Tcl,Lib) \
    -DTK_LIBRARY:FILEPATH=$::SBuild(Tk,Lib) \
    -DTCL_TCLSH:FILEPATH=$::SBuild(Tcl,tclsh) \
    ../VTK
}


proc VTK-Configure-Darwin {} {
  Debug "Configure VTK"
  global SBuild Plugin
  file mkdir VTK-build
  cd VTK-build
  set OpenGLString "-framework OpenGL;/usr/X11R6/lib/libGL.dylib"
  ExecuteCommand $SBuild(CMake) \
    -G$SBuild(Generator) \
    -DCMAKE_BUILD_TYPE:STRING=$::SBuild(BuildType) \
    -DCMAKE_CXX_FLAGS_DEBUG:STRING=$::SBuild(CMakeCXXFlagsDebug) \
    -DBUILD_SHARED_LIBS:BOOL=ON \
    -DCMAKE_SKIP_RPATH:BOOL=ON \
    -DCMAKE_CXX_COMPILER:STRING=$::SBuild(CompilerPath)/$SBuild(Compiler) \
    -DCMAKE_CXX_COMPILER_FULLPATH:FILEPATH=$::SBuild(CompilerPath)/$SBuild(Compiler) \
    -DBUILD_TESTING:BOOL=OFF \
    -DVTK_USE_CARBON:BOOL=OFF \
    -DVTK_USE_X:BOOL=ON \
    -DVTK_WRAP_TCL:BOOL=ON \
    -DVTK_USE_HYBRID:BOOL=ON \
    -DVTK_USE_PATENTED:BOOL=ON \
    -DTCL_INCLUDE_PATH:PATH=$SBuild(Tcl,IncludePath) \
    -DTK_INCLUDE_PATH:PATH=$SBuild(Tcl,IncludePath) \
    -DTCL_LIBRARY:FILEPATH=$::SBuild(Tcl,Lib) \
    -DTK_LIBRARY:FILEPATH=$::SBuild(Tk,Lib) \
    -DTCL_TCLSH:FILEPATH=$::SBuild(Tcl,tclsh) \
    -DOPENGL_gl_LIBRARY:STRING=$OpenGLString \
    "-DCMAKE_SHARED_LINKER_FLAGS:STRING=-dylib_file /System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib" \
    "-DCMAKE_EXE_LINKER_FLAGS:STRING=-dylib_file /System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib" \
    ../VTK

}

proc VTK-Configure-Linux {} {
  Debug "Configure VTK"
  global SBuild Plugin
  if { $::tcl_platform(machine) == "x86_64" } {
  file mkdir VTK-build
  cd VTK-build
  ExecuteCommand $SBuild(CMake) \
    -G$SBuild(Generator) \
    -DCMAKE_BUILD_TYPE:STRING=$::SBuild(BuildType) \
    -DCMAKE_CXX_FLAGS_DEBUG:STRING=$::SBuild(CMakeCXXFlagsDebug) \
    -DBUILD_SHARED_LIBS:BOOL=ON \
    -DCMAKE_SKIP_RPATH:BOOL=ON \
    -DCMAKE_CXX_COMPILER:STRING=$::SBuild(CompilerPath)/$SBuild(Compiler) \
    -DCMAKE_CXX_COMPILER_FULLPATH:FILEPATH=$::SBuild(CompilerPath)/$SBuild(Compiler) \
    -DBUILD_TESTING:BOOL=OFF \
    -DVTK_USE_CARBON:BOOL=OFF \
    -DVTK_USE_X:BOOL=ON \
    -DVTK_WRAP_TCL:BOOL=ON \
    -DVTK_USE_HYBRID:BOOL=ON \
    -DVTK_USE_PATENTED:BOOL=ON \
    -DTCL_INCLUDE_PATH:PATH=$SBuild(Tcl,IncludePath) \
    -DTK_INCLUDE_PATH:PATH=$SBuild(Tcl,IncludePath) \
    -DTCL_LIBRARY:FILEPATH=$::SBuild(Tcl,Lib) \
    -DTK_LIBRARY:FILEPATH=$::SBuild(Tk,Lib) \
    -DTCL_TCLSH:FILEPATH=$::SBuild(Tcl,tclsh) \
    -DOPENGL_INCLUDE_DIR:PATH=/usr/include \
    -DOPENGL_gl_LIBRARY:FILEPATH=/usr/lib64/libGL.so \
    -DOPENGL_glu_LIBRARY:FILEPATH=/usr/lib64/libGLU.so \
    -DX11_X11_LIB:FILEPATH=/usr/X11R6/lib64/libX11.a \
    -DX11_Xext_LIB:FILEPATH=/usr/X11R6/lib64/libXext.a \
    -DCMAKE_MODULE_LINKER_FLAGS:STRING=-L/usr/X11R6/lib64 \
    -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON \
    -DVTK_USE_64BIT_IDS:BOOL=ON \
    ../VTK
  } else {
    VTK-Configure
  }
}

proc VTK-Configure {} {
  global SBuild Plugin
  file mkdir VTK-build
  cd VTK-build
  ExecuteCommand $SBuild(CMake) \
    -G$SBuild(Generator) \
    -DCMAKE_BUILD_TYPE:STRING=$::SBuild(BuildType) \
    -DCMAKE_CXX_FLAGS_DEBUG:STRING=$::SBuild(CMakeCXXFlagsDebug) \
    -DBUILD_SHARED_LIBS:BOOL=ON \
    -DCMAKE_SKIP_RPATH:BOOL=ON \
    -DCMAKE_CXX_COMPILER:STRING=$::SBuild(CompilerPath)/$SBuild(Compiler) \
    -DCMAKE_CXX_COMPILER_FULLPATH:FILEPATH=$::SBuild(CompilerPath)/$SBuild(Compiler) \
    -DBUILD_TESTING:BOOL=OFF \
    -DVTK_USE_CARBON:BOOL=OFF \
    -DVTK_USE_X:BOOL=ON \
    -DVTK_WRAP_TCL:BOOL=ON \
    -DVTK_USE_HYBRID:BOOL=ON \
    -DVTK_USE_PATENTED:BOOL=ON \
    -DTCL_INCLUDE_PATH:PATH=$SBuild(Tcl,IncludePath) \
    -DTK_INCLUDE_PATH:PATH=$SBuild(Tcl,IncludePath) \
    -DTCL_LIBRARY:FILEPATH=$::SBuild(Tcl,Lib) \
    -DTK_LIBRARY:FILEPATH=$::SBuild(Tk,Lib) \
    -DTCL_TCLSH:FILEPATH=$::SBuild(Tcl,tclsh) \
    ../VTK
}


proc VTK-Build-Windows {} {
  global SBuild
  cd VTK-build
  ExecuteCommand $SBuild(Make) VTK.SLN /build  $SBuild(BuildType)
}

proc VTK-Build {} {
  Debug "Building VTK"
  global SBuild
  cd VTK-build
  eval ExecuteCommand $SBuild(Make)
}

proc VTK-Clean {} {
  Debug "Cleaning VTK"
}



