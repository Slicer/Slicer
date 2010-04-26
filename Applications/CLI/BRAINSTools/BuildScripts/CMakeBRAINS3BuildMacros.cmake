include(ExternalProject)

#
# argv1 == program name
# argv2 == src variable name
# argv2 == xml name
macro(BRAINSGENERATEMODULESCRIPT progname)
  #
  # Next, do the configure thing.
  #
  # get the directory to throw the dummy into
  # message(STATUS "PROGRAM NAME ${progname}")
  get_target_property(BGCbin_dir ${progname} RUNTIME_OUTPUT_DIRECTORY)

  set(BGCbin_name ${progname})
  IF(APPLE)
    SET(OS_VARNAME_FOR_LIBRARY_PATH "DYLD_LIBRARY_PATH")
  ELSE(APPLE)
    SET(OS_VARNAME_FOR_LIBRARY_PATH "LD_LIBRARY_PATH")
  ENDIF(APPLE)

  #  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/BuildScripts/Module_Dummy.in ${BGCbin_dir}/Modules/${progname} )

FILE(WRITE ${BGCbin_dir}/Modules/${progname}
"#!/usr/bin/tclsh
catch {set script [info script]}
catch {set script [file normalize \$script]}
catch {set execdir [file dirname [file dirname \$script ]]}
set env(${OS_VARNAME_FOR_LIBRARY_PATH}) [ exec \$execdir/brains3_setup.sh ${OS_VARNAME_FOR_LIBRARY_PATH} ]
set command \"\$execdir/${BGCbin_name} \$argv\"
set fp [ open \"| \$command |& cat\" \"r\"]
while { ![eof \$fp ] } {
    gets \$fp line
    puts \$line
}
if { [catch \"close \$fp\" res] } {
    exit [ lindex \$errorCode 2 ]
} else {
    exit 0
}
"
)


  install(PROGRAMS ${BGCbin_dir}/Modules/${progname}
    DESTINATION bin/Modules PERMISSIONS WORLD_EXECUTE)
endmacro(BRAINSGENERATEMODULESCRIPT)

## A macro to create executables for Slicer or BRAINS3
if(NOT CONFIGUREBRAINSORSLICERPROPERTIES)
macro(CONFIGUREBRAINSORSLICERPROPERTIES PROGNAME PROGCLI PROGSOURCES EXTRA_LIBS)

  find_package(GenerateCLP NO_MODULE REQUIRED)
  include(${GenerateCLP_USE_FILE})

get_filename_component(TMP_FILENAME ${PROGCLI} NAME_WE)
set(PROGCLI_HEADER "${CMAKE_CURRENT_BINARY_DIR}/${TMP_FILENAME}CLP.h")


set(CLP_SOURCES ${PROGSOURCES})
if(EXISTS  ${BRAINS_CMAKE_HELPER_DIR}/BRAINSLogo.h)
  GENERATECLP(CLP_SOURCES ${PROGCLI} ${BRAINS_CMAKE_HELPER_DIR}/BRAINSLogo.h)
else()
  GENERATECLP(CLP_SOURCES ${PROGCLI} )
endif()
ADD_EXECUTABLE( ${PROGNAME} ${CLP_SOURCES} ${PROGCLI_HEADER})
target_link_libraries (${PROGNAME} BRAINSCommonLib ITKAlgorithms ITKIO ITKBasicFilters ${OPTIONAL_DEBUG_LINK_LIBRARIES} ${EXTRA_LIBS} )


if (Slicer3_SOURCE_DIR)
  slicer3_set_plugins_output_path(${PROGNAME})
  add_library(${PROGNAME}Lib SHARED ${CLP_SOURCES} ${PROGCLI_HEADER})
  slicer3_set_plugins_output_path(${PROGNAME}Lib)
  set_target_properties (${PROGNAME}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
  target_link_libraries (${PROGNAME}Lib BRAINSCommonLib ITKAlgorithms ITKIO ITKBasicFilters ${OPTIONAL_DEBUG_LINK_LIBRARIES} ${EXTRA_LIBS} )
  set(TARGETS ${PROGNAME}Lib ${PROGNAME})
  slicer3_install_plugins(${TARGETS})
else (Slicer3_SOURCE_DIR)
  IF(BRAINS_BUILD)
    BRAINSGENERATEMODULESCRIPT(${PROGNAME})
  ENDIF(BRAINS_BUILD)
  INSTALL(TARGETS ${PROGNAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)
endif (Slicer3_SOURCE_DIR)

endmacro(CONFIGUREBRAINSORSLICERPROPERTIES PROGNAME PROGCLI PROGSOURCES)
endif(NOT CONFIGUREBRAINSORSLICERPROPERTIES)

if(0)
#-----------------------------------------------------------------------------
# Get and build BRAINSCommonLib
if(NOT FINDORBUILD_COMMON_BRAINSCOMMONLIB)
macro(FINDORBUILD_COMMON_BRAINSCOMMONLIB)
IF(NOT COMMON_BRAINSCOMMONLIB_BINARY_DIR)
set(proj BRAINSCommonLibExternal)
ExternalProject_Add(${proj}
  SVN_REPOSITORY "https://www.nitrc.org/svn/brains/BRAINSCommonLib/trunk"
  SOURCE_DIR BRAINSCommonLib
  BINARY_DIR BRAINSCommonLib-build
  CMAKE_GENERATOR ${gen}
  CMAKE_ARGS
    -DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
    -DCMAKE_C_FLAGS:STRING=${CMAKE_C_FLAGS}
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    -DBUILD_EXAMPLES:BOOL=OFF
    -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}
    -DBUILD_TESTING:BOOL=${BUILD_TESTING}
    -DCMAKE_SKIP_RPATH:BOOL=ON
    -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
    -DITK_DIR:PATH=${ITK_DIR}
  INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
)
SET(COMMON_BRAINSCOMMONLIB_SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/BRAINSCommonLib)
SET(COMMON_BRAINSCOMMONLIB_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/BRAINSCommonLib-build)
ENDIF(NOT COMMON_BRAINSCOMMONLIB_BINARY_DIR)
endmacro(FINDORBUILD_COMMON_BRAINSCOMMONLIB)
endif(NOT FINDORBUILD_COMMON_BRAINSCOMMONLIB)
endif(0)


#-----------------------------------------------------------------------------
# Build the optional DEBUGIMAGEVIEWER
if(NOT SETOPTIONALDEBUGIMAGEVIEWER)
macro(SETOPTIONALDEBUGIMAGEVIEWER)
if(BRAINS_BUILD)
  option(USE_DEBUG_IMAGE_VIEWER "Use the DEBUG_IMAGE_VIEWER for debugging" ON)
else(BRAINS_BUILD)
  option(USE_DEBUG_IMAGE_VIEWER "Use the DEBUG_IMAGE_VIEWER for debugging" OFF)
endif(BRAINS_BUILD)

mark_as_advanced(USE_DEBUG_IMAGE_VIEWER)
set(OPTIONAL_DEBUG_LINK_LIBRARIES) ## Set it to empty as the default
if( USE_DEBUG_IMAGE_VIEWER )
   if(NOT KWWidgets_SOURCE_DIR)
     find_package(KWWidgets REQUIRED)
     include(${KWWidgets_USE_FILE})
   endif(NOT KWWidgets_SOURCE_DIR)
   add_definitions(-DUSE_DEBUG_IMAGE_VIEWER)
   find_path(DEBUG_IMAGE_VIEWER_INCLUDE_DIR DebugImageViewerClient.h ${CMAKE_INSTALL_PREFIX}/include)
   include_directories(${DEBUG_IMAGE_VIEWER_INCLUDE_DIR})
   set(OPTIONAL_DEBUG_LINK_LIBRARIES ${KWWidgets_LIBRARIES})
endif( USE_DEBUG_IMAGE_VIEWER )
endmacro(SETOPTIONALDEBUGIMAGEVIEWER)
endif(NOT SETOPTIONALDEBUGIMAGEVIEWER)

