project(GenerateCLP)

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

# --------------------------------------------------------------------------
# Prerequisites
# --------------------------------------------------------------------------
find_package(ModuleDescriptionParser REQUIRED)
include(${ModuleDescriptionParser_USE_FILE})

find_package(TCLAP REQUIRED)
include(${TCLAP_USE_FILE})

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

# --------------------------------------------------------------------------
# Option(s)
# --------------------------------------------------------------------------
if(NOT DEFINED BUILD_SHARED_LIBS)
  option(BUILD_SHARED_LIBS "Build with shared libraries." ON)
endif()

# --------------------------------------------------------------------------
# Build and link executable
# --------------------------------------------------------------------------
set(GENERATECLP_SOURCE 
  GenerateCLP.cxx
  )
add_executable(GenerateCLP ${GENERATECLP_SOURCE})

target_link_libraries(GenerateCLP
  # Appearently windows does not like static libs mixed with shared libs ModuleDescriptionParser-static
  # A different solution will have to be investigated for makeing GenerateCLP work without
  # shared libs.
  ModuleDescriptionParser
  itksys
  ITKEXPAT)

include(GenerateGenerateCLPConfig.cmake)

# --------------------------------------------------------------------------
# Install Rules
# --------------------------------------------------------------------------

if(NOT DEFINED GenerateCLP_INSTALL_BIN_DIR)
  SET(GenerateCLP_INSTALL_BIN_DIR bin)
endif()

install(TARGETS GenerateCLP RUNTIME 
  DESTINATION ${GenerateCLP_INSTALL_BIN_DIR} COMPONENT Development
  )
install(FILES ${GenerateCLP_BINARY_DIR}/GenerateCLPConfig.cmake_install
  DESTINATION lib/GenerateCLP
  COMPONENT Development
  RENAME GenerateCLPConfig.cmake
  )
install(FILES ${GenerateCLP_BINARY_DIR}/UseGenerateCLP.cmake_install
  DESTINATION lib/GenerateCLP
  COMPONENT Development
  RENAME UseGenerateCLP.cmake
  )

# --------------------------------------------------------------------------
# Launcher
# --------------------------------------------------------------------------
# Enable shared link forwarding support if needed.
# This is required so that GenerateCLP can be run from an installed tree,
# where the binary is actually not in the same directory as the shared
# libraries it depends on.
#
set(GenerateCLP_FORWARD_DIR_BUILD "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
set(GenerateCLP_FORWARD_DIR_INSTALL ".")
if(WIN32)
  set(GenerateCLP_FORWARD_PATH_BUILD 
    "\"${GenerateCLP_FORWARD_DIR_BUILD}\" CONFIG_DIR_POST,\"${ITK_DIR}/bin\" CONFIG_DIR_POST")
  set(GenerateCLP_FORWARD_PATH_INSTALL "\"\"")
else()
  set(GenerateCLP_FORWARD_PATH_BUILD 
    "\"${GenerateCLP_FORWARD_DIR_BUILD}\",\"${ITK_DIR}/bin\"")
  set(GenerateCLP_FORWARD_PATH_INSTALL
    "\"${GenerateCLP_DIR}\",\"${ITK_DIR}\",\"${ModuleDescriptionParser_DIR}\"")
endif()
set(GenerateCLP_FORWARD_EXE GenerateCLP)

if(NOT WIN32)
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/GenerateCLPLauncher.c.in
    ${CMAKE_CURRENT_BINARY_DIR}/GenerateCLPLauncher.c
    @ONLY)
  add_executable(GenerateCLPLauncher
    ${CMAKE_CURRENT_BINARY_DIR}/GenerateCLPLauncher.c)
  add_dependencies(GenerateCLPLauncher GenerateCLP)

  install(TARGETS GenerateCLPLauncher RUNTIME 
    DESTINATION bin COMPONENT Development
    )
endif()

# --------------------------------------------------------------------------
if(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
endif()
