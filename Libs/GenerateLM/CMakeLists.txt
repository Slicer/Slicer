project(GenerateLM)

#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.2)
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# See http://cmake.org/cmake/help/cmake-2-8-docs.html#section_Policies for details
#-----------------------------------------------------------------------------

SET(project_policies
  CMP0001 # NEW: CMAKE_BACKWARDS_COMPATIBILITY should no longer be used.
  CMP0002 # NEW: Logical target names must be globally unique.
  CMP0003 # NEW: Libraries linked via full path no longer produce linker search paths.
  CMP0004 # NEW: Libraries linked may NOT have leading or trailing whitespace.
  CMP0005 # NEW: Preprocessor definition values are now escaped automatically.
  CMP0006 # NEW: Installing MACOSX_BUNDLE targets requires a BUNDLE DESTINATION.
  CMP0007 # NEW: List command no longer ignores empty elements.
  CMP0008 # NEW: Libraries linked by full-path must have a valid library file name.
  CMP0009 # NEW: FILE GLOB_RECURSE calls should not follow symlinks by default.
  CMP0010 # NEW: Bad variable reference syntax is an error.
  CMP0011 # NEW: Included scripts do automatic cmake_policy PUSH and POP.
  CMP0012 # NEW: if() recognizes numbers and boolean constants.
  CMP0013 # NEW: Duplicate binary directories are not allowed.
  CMP0014 # NEW: Input directories must have CMakeLists.txt
  )
FOREACH(policy ${project_policies})
  IF(POLICY ${policy})
    CMAKE_POLICY(SET ${policy} NEW)
  ENDIF()
ENDFOREACH()


find_package(GenerateCLP REQUIRED)
if(GenerateCLP_FOUND)
  include(${GenerateCLP_USE_FILE})
endif(GenerateCLP_FOUND)

find_package(LoadableModule REQUIRED)
if(LoadableModule_FOUND)
  include(${LoadableModule_USE_FILE})
endif(LoadableModule_FOUND)

find_package(ITK REQUIRED)
if(ITK_FOUND)
  include(${ITK_USE_FILE})
endif(ITK_FOUND)

find_package(VTK REQUIRED)
if(VTK_FOUND)
  include(${VTK_USE_FILE})
endif(VTK_FOUND)

if(NOT DEFINED BUILD_SHARED_LIBS)
  option(BUILD_SHARED_LIBS "Build with shared libraries." ON)
endif(NOT DEFINED BUILD_SHARED_LIBS)

set(GENERATELM_SOURCE GenerateLM.cxx)
generateclp(GENERATELM_SOURCE GenerateLM.xml)
add_executable(GenerateLM ${GENERATELM_SOURCE})
get_target_property(GenerateLM_EXE_PATH GenerateLM LOCATION)
target_link_libraries(GenerateLM
  LoadableModule
  itksys
  ITKEXPAT)

if(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
endif(BUILD_TESTING)

include(GenerateGenerateLMConfig.cmake)

install(TARGETS GenerateLM RUNTIME
  DESTINATION bin COMPONENT Development
  )
install(FILES ${GenerateLM_BINARY_DIR}/GenerateLMConfig.cmake_install
  DESTINATION lib/GenerateLM
  COMPONENT Development
  RENAME GenerateLMConfig.cmake
  )
install(FILES ${GenerateLM_BINARY_DIR}/UseGenerateLM.cmake_install
  DESTINATION lib/GenerateLM
  COMPONENT Development
  RENAME UseGenerateLM.cmake
  )

# --------------------------------------------------------------------------
# Enable shared link forwarding support if needed.
# This is required so that GenerateLM can be run from an installed tree,
# where the binary is actually not in the same directory as the shared
# libraries it depends on.
#
set(GenerateLM_FORWARD_DIR_BUILD "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
set(GenerateLM_FORWARD_DIR_INSTALL ".")
if(WIN32)
  set(GenerateLM_FORWARD_PATH_BUILD
    "\"${GenerateLM_FORWARD_DIR_BUILD}\" CONFIG_DIR_POST,\"${VTK_DIR}/bin\" CONFIG_DIR_POST,\"${ITK_DIR}/bin\" CONFIG_DIR_POST")
  set(GenerateLM_FORWARD_PATH_INSTALL "\"\"")
else(WIN32)
  set(GenerateLM_FORWARD_PATH_BUILD
    "\"${GenerateLM_FORWARD_DIR_BUILD}\",\"${VTK_DIR}/bin\",\"${ITK_DIR}/bin\"")
  set(GenerateLM_FORWARD_PATH_INSTALL
    "\"../lib/GenerateLM\",\"../lib/vtk-${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION}\",\"../lib/InsightToolkit\",\"../lib/LoadableModule\"")
endif(WIN32)
set(GenerateLM_FORWARD_EXE GenerateLM)

if(NOT WIN32)
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/GenerateLMLauncher.c.in
    ${CMAKE_CURRENT_BINARY_DIR}/GenerateLMLauncher.c
    @ONLY)
  add_executable(GenerateLMLauncher
    ${CMAKE_CURRENT_BINARY_DIR}/GenerateLMLauncher.c)
  add_dependencies(GenerateLMLauncher GenerateLM)

  install(TARGETS GenerateLMLauncher RUNTIME
    DESTINATION bin COMPONENT Development
    )
endif(NOT WIN32)

