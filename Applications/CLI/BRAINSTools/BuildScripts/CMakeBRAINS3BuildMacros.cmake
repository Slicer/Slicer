include(ExternalProject)

#  This is a hack to get similar behavior in both Slicer3 and Slicer4 for building BRAINSFit
if (Slicer3_SOURCE_DIR)
  set(Slicer_SOURCE_DIR "${Slicer3_SOURCE_DIR}" CACHE INTERNAL "Needed to get Slicer3 and Slicer4 to have same behavior" FORCE)
endif (Slicer3_SOURCE_DIR)

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
  if(APPLE)
    set(OS_VARNAME_FOR_LIBRARY_PATH "DYLD_LIBRARY_PATH")
  else(APPLE)
    set(OS_VARNAME_FOR_LIBRARY_PATH "LD_LIBRARY_PATH")
  endif(APPLE)

  configure_file(${BRAINS_CMAKE_HELPER_DIR}/Module_Dummy.in ${BGCbin_dir}/Modules/${progname} )

# file(WRITE ${BGCbin_dir}/Modules/${progname}
# "#!/usr/bin/tclsh
# catch {set script [info script]}
# catch {set script [file normalize \$script]}
# catch {set execdir [file dirname [file dirname \$script ]]}
# set env(${OS_VARNAME_FOR_LIBRARY_PATH}) [ exec \$execdir/brains3_setup.sh ${OS_VARNAME_FOR_LIBRARY_PATH} ]
# set command \"\$execdir/${BGCbin_name} \$argv\"
# set fp [ open \"| \$command |& cat\" \"r\"]
# while { ![eof \$fp ] } {
#     gets \$fp line
#     puts \$line
# }
# if { [catch \"close \$fp\" res] } {
#     exit [ lindex \$errorCode 2 ]
# } else {
#     exit 0
# }
# "
# )


  install(PROGRAMS ${BGCbin_dir}/Modules/${progname}
    DESTINATION bin/Modules PERMISSIONS WORLD_EXECUTE)
endmacro(BRAINSGENERATEMODULESCRIPT)


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

#-----------------------------------------------------------------------------
#-----------------------------------------------------------------------------
#-----------------------------------------------------------------------------
#-----------------------------------------------------------------------------
#-----------------------------------------------------------------------------
#-----------------------------------------------------------------------------
#-----------------------------------------------------------------------------
## A macro to create executables for Slicer or BRAINS3
if(NOT CONFIGUREBRAINSORSLICERPROPERTIES)
  macro(CONFIGUREBRAINSORSLICERPROPERTIES PROGNAME PROGCLI PROGSOURCES LIBSOURCES ENTRYPOINTNAME EXTRA_LIBS)

  find_package(SlicerExecutionModel REQUIRED GenerateCLP)

  find_package(GenerateCLP NO_MODULE REQUIRED)
  include(${GenerateCLP_USE_FILE})

  get_filename_component(TMP_FILENAME ${PROGCLI} NAME_WE)
  set(PROGCLI_HEADER "${CMAKE_CURRENT_BINARY_DIR}/${TMP_FILENAME}CLP.h")

  set(CLP_SOURCES ${PROGSOURCES} ${LIBSOURCES})
  set(CLP_PRIMARY_SOURCES ${LIBSOURCES})
  if(EXISTS  ${BRAINS_CMAKE_HELPER_DIR}/BRAINSLogo.h)
    GENERATECLP(CLP_SOURCES ${PROGCLI} ${BRAINS_CMAKE_HELPER_DIR}/BRAINSLogo.h)
  else()
    GENERATECLP(CLP_SOURCES ${PROGCLI} )
  endif()

  add_executable( ${PROGNAME} ${CLP_SOURCES} ${PROGCLI_HEADER})

  if(WIN32)
    set(BRAINS_ITK_LIBS "")
  else(WIN32)
    set(BRAINS_ITK_LIBS ITKAlgorithms ITKIO ITKBasicFilters)
  endif(WIN32)
  target_link_libraries (${PROGNAME} BRAINSCommonLib ${BRAINS_ITK_LIBS} ${OPTIONAL_DEBUG_LINK_LIBRARIES} ${EXTRA_LIBS} )

  if (Slicer_SOURCE_DIR)
    ### If building as part of the Slicer_SOURCE_DIR, then only build the shared object, and not the command line program.

    add_library(${PROGNAME}Lib SHARED ${CLP_PRIMARY_SOURCES} ${PROGCLI_HEADER})
    set_target_properties (${PROGNAME}Lib PROPERTIES COMPILE_FLAGS "-D${ENTRYPOINTNAME}=ModuleEntryPoint")
    slicer3_set_plugins_output_path(${PROGNAME}Lib)
    target_link_libraries (${PROGNAME}Lib BRAINSCommonLib ${BRAINS_ITK_LIBS} ${OPTIONAL_DEBUG_LINK_LIBRARIES} ${EXTRA_LIBS} )

    # install each target in the production area (where it would appear in an
    # installation) and install each target in the developer area (for running
    # from a build)
    slicer3_set_plugins_output_path(${PROGNAME})
    set(TARGETS ${PROGNAME}Lib ${PROGNAME})
    slicer3_install_plugins(${TARGETS})
  else (Slicer_SOURCE_DIR)
    ### If building outside of Slicer3, then only build the command line executable.
    if(BRAINS_BUILD)
      BRAINSGENERATEMODULESCRIPT(${PROGNAME})
    endif(BRAINS_BUILD)
    install(TARGETS ${PROGNAME}
      RUNTIME DESTINATION bin
      LIBRARY DESTINATION lib
      ARCHIVE DESTINATION lib)
  endif (Slicer_SOURCE_DIR)

endmacro(CONFIGUREBRAINSORSLICERPROPERTIES PROGNAME PROGCLI PROGSOURCES LIBSOURCES)
endif(NOT CONFIGUREBRAINSORSLICERPROPERTIES)
#-----------------------------------------------------------------------------
#-----------------------------------------------------------------------------
#-----------------------------------------------------------------------------
#-----------------------------------------------------------------------------
#-----------------------------------------------------------------------------
#-----------------------------------------------------------------------------
#-----------------------------------------------------------------------------
## A macro to create CLP dependant libraries for Slicer or BRAINS3
if(NOT CONFIGUREBRAINSORSLICERLIBRARY)
macro(CONFIGUREBRAINSORSLICERLIBRARY LIBNAME LIBCLI LIBSOURCES EXTRA_LIBS)

  find_package(SlicerExecutionModel REQUIRED GenerateCLP)

  find_package(GenerateCLP NO_MODULE REQUIRED)
  include(${GenerateCLP_USE_FILE})

  get_filename_component(TMP_FILENAME ${LIBCLI} NAME_WE)
  set(LIBCLI_HEADER "${CMAKE_CURRENT_BINARY_DIR}/${TMP_FILENAME}CLP.h")

  set(CLP_SOURCES ${LIBSOURCES})
  if(EXISTS  ${BRAINS_CMAKE_HELPER_DIR}/BRAINSLogo.h)
    GENERATECLP(CLP_SOURCES ${LIBCLI} ${BRAINS_CMAKE_HELPER_DIR}/BRAINSLogo.h)
  else()
    GENERATECLP(CLP_SOURCES ${LIBCLI} )
  endif()
  add_library( ${LIBNAME} ${CLP_SOURCES} ${LIBCLI_HEADER})
  target_link_libraries (${LIBNAME} BRAINSCommonLib ITKAlgorithms ITKIO ITKBasicFilters ${OPTIONAL_DEBUG_LINK_LIBRARIES} ${EXTRA_LIBS} )

  if (Slicer_SOURCE_DIR)
    ### If building as part of the Slicer_SOURCE_DIR, then only build the shared object, and not the command line program.
    # install each target in the production area (where it would appear in an
    # installation) and install each target in the developer area (for running
    # from a build)
    slicer3_set_plugins_output_path(${LIBNAME})
    set(TARGETS ${LIBNAME})
    slicer3_install_plugins(${TARGETS})
  else (Slicer_SOURCE_DIR)
    ### If building outside of Slicer3, then only build the command line executable.
    if(BRAINS_BUILD)
      BRAINSGENERATEMODULESCRIPT(${LIBNAME})
    endif(BRAINS_BUILD)
    install(TARGETS ${LIBNAME}
      RUNTIME DESTINATION bin
      LIBRARY DESTINATION lib
      ARCHIVE DESTINATION lib)
  endif (Slicer_SOURCE_DIR)

endmacro(CONFIGUREBRAINSORSLICERLIBRARY LIBNAME LIBCLI LIBSOURCES)
endif(NOT CONFIGUREBRAINSORSLICERLIBRARY)
