package provide SlicerBuildCommands 1.0

set ::Slicer(SlicerTag) "http://www.na-mic.org/svn/Slicer3/trunk"

proc Slicer-Update {} {
  global SBuild Slicer
  Debug "Running Slicer-Update"
  ExecuteCommand $SBuild(SVNCommand) co $Slicer(SlicerTag) $SBuild(SlicerSourceDir)
}

# Platform specific settings
proc Slicer-PlatformConfigure {} {Error "Should never call generic Slicer-PlatformConfigure" }
proc Slicer-PlatformConfigure-Darwin {} {
  global Slicer SBuild
  set SBuild(Generator) "Unix Makefiles"
  set SBuild(CompilerPath) "/usr/bin"
  set SBuild(Compiler) "g++"
  set SBuild(BuildJobs) [expr [exec sysctl -n hw.ncpu] * 2]
  set SBuild(Make) "make"
  set SBuild(ParallelMake) "-j $SBuild(BuildJobs)"
}


proc Slicer-Configure {} {
  global SBuild Slicer
  Debug "Running Slicer-Configure"
  ExecutePluginMethod Slicer PlatformConfigure
  # Let each plugin configure itself first
  set Slicer(CMakeArguments) ""
  SortPlugins
  foreach plugin $SBuild(Plugins) {
    ExecutePluginMethod $plugin ConfigureSlicer
  }

  # Now, run CMake
  file mkdir $SBuild(SlicerBuildDir)
  cd $SBuild(SlicerBuildDir)
  Debug "Running CMake on Slicer"

  set cmd [list $SBuild(CMake) \
             -G$SBuild(Generator) \
             -DMAKECOMMAND:STRING=$SBuild(Make) \
             -DCMAKE_CXX_COMPILER:STRING=$SBuild(CompilerPath)/$SBuild(Compiler) \
             -DCMAKE_CXX_COMPILER_FULLPATH:FILEPATH=$SBuild(CompilerPath)/$SBuild(Compiler) \
             -DCMAKE_BUILD_TYPE=$SBuild(BuildType) \
             -DCMAKE_CXX_FLAGS_DEBUG:STRING=$SBuild(CMakeCXXFlagsDebug) ]
  set cmd [concat $cmd $Slicer(CMakeArguments)]
  lappend cmd $SBuild(SlicerSourceDir)
  eval ExecuteCommand $cmd
  if { 0 } {
    -DITK_DIR:FILEPATH=$ITK_BINARY_PATH \
        -DKWWidgets_DIR:FILEPATH=$SLICER_LIB/KWWidgets-build \
        -DTEEM_DIR:FILEPATH=$SLICER_LIB/teem-build \
        -DIGSTK_DIR:FILEPATH=$SLICER_LIB/IGSTK-build \
        -DSandBox_DIR:FILEPATH=$SLICER_LIB/NAMICSandBox \
        -DSlicer3_VERSION_PATCH:STRING=$::GETBUILDTEST(version-patch) \
        -DCPACK_GENERATOR:STRING=$::GETBUILDTEST(cpack-generator) \
        -DCPACK_PACKAGE_FILE_NAME:STRING=$::GETBUILDTEST(binary-filename) \
        -DUSE_PYTHON=$BuildPython \
        -DPYTHON_INCLUDE_PATH:PATH=$::SLICER_LIB/python-build/include/python2.5 \
        -DPYTHON_LIBRARY:FILEPATH=$::SLICER_LIB/python-build/lib/libpython2.5$::GETBUILDTEST(shared-lib-extension) \
        -DUSE_IGSTK=$::IGSTK \
        -DUSE_OPENTRACKER=$::OPENTRACKER \
        -DOT_VERSION_13=$::OT_VERSION \
        -DOT_LIB_DIR:FILEPATH=$::OT_LIB_DIR \
        -DOT_INC_DIR:FILEPATH=$::OT_INC_DIR \
        -DNAVITRACK_INCLUDE_DIR:FILEPATH=$SLICER_LIB/NaviTrack/include \
        -DNAVITRACK_BINARY_DIR:FILEPATH=$SLICER_LIB/NaviTrack-build/$VTK_BUILD_SUBDIR/ \
        -Ddcmtk_SOURCE_DIR:FILEPATH=$SLICER_LIB/dcmtk \
        -DBatchMake_DIR:FILEPATH=$SLICER_LIB/BatchMake-build \
        -DUSE_BatchMake=ON \
        -DSLICERLIBCURL_DIR:FILEPATH=$SLICER_LIB/cmcurl-build \
        -DUSE_MIDAS=ON \
        $SLICER_HOME
  }
}

proc Slicer-Build {} {
  global SBuild
  Debug "Running Slicer-Build"
  cd $SBuild(SlicerBuildDir)
  eval ExecuteCommand $SBuild(Make) $SBuild(ParallelMake)
}

proc Slicer-Test {} {
  global SBuild
  Debug "Running Slicer-Test"
}


proc Slicer-All {} {
  global SBuild
  Debug "Running Slicer-All"
  Slicer-Update
  Slicer-Configure
  Slicer-Test
}
