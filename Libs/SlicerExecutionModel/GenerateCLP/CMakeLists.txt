project(GenerateCLP)

#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.2)
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# See http://cmake.org/cmake/help/cmake-2-8-docs.html#section_Policies for details
#-----------------------------------------------------------------------------
if(POLICY CMP0016)
  cmake_policy(SET CMP0016 NEW)
endif()
if(POLICY CMP0017)
  cmake_policy(SET CMP0017 OLD)
endif()

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
if(NOT DEFINED GENERATECLP_USE_MD5)
  option(GENERATECLP_USE_MD5 "Enabling that option will teach GenerateCLP to overwrite output file only if needed. Make sure to build ITK with KWSYS_USE_MD5 enabled." OFF)
endif()

# --------------------------------------------------------------------------
# Configure header(s)
# --------------------------------------------------------------------------
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/GenerateCLPConfig.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/GenerateCLPConfig.h
  )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  )

# --------------------------------------------------------------------------
# Build executable
# --------------------------------------------------------------------------
set(executable_name GenerateCLP)
set(GENERATECLP_SOURCE
  GenerateCLP.cxx
  )
add_executable(${executable_name} ${GENERATECLP_SOURCE})

target_link_libraries(${executable_name}
  # Appearently windows does not like static libs mixed with shared libs ModuleDescriptionParser-static
  # A different solution will have to be investigated for makeing GenerateCLP work without
  # shared libs.
  ModuleDescriptionParser
  itksys
  ITKEXPAT)

include(GenerateGenerateCLPConfig.cmake)

# --------------------------------------------------------------------------
# Export target
# --------------------------------------------------------------------------
if(NOT DEFINED ${PROJECT_NAME}_EXPORT_FILE)
  set(${PROJECT_NAME}_EXPORT_FILE ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake)
endif()
export(TARGETS ${executable_name} APPEND FILE ${${PROJECT_NAME}_EXPORT_FILE})

# --------------------------------------------------------------------------
# Install executable
# --------------------------------------------------------------------------
if(NOT DEFINED GenerateCLP_INSTALL_BIN_DIR)
  set(GenerateCLP_INSTALL_BIN_DIR bin)
endif()
if(NOT DEFINED ${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT)
  set(${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT ON)
endif()
if(NOT ${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT)
  install(TARGETS GenerateCLP RUNTIME
    DESTINATION ${GenerateCLP_INSTALL_BIN_DIR} COMPONENT Development
    )
endif()

# --------------------------------------------------------------------------
# Install support files
# --------------------------------------------------------------------------
if(NOT ${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT)
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
endif()

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
  # --------------------------------------------------------------------------
  # Build launcher
  # --------------------------------------------------------------------------
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/GenerateCLPLauncher.c.in
    ${CMAKE_CURRENT_BINARY_DIR}/GenerateCLPLauncher.c
    @ONLY)
  add_executable(GenerateCLPLauncher
    ${CMAKE_CURRENT_BINARY_DIR}/GenerateCLPLauncher.c)
  add_dependencies(GenerateCLPLauncher GenerateCLP)

  # --------------------------------------------------------------------------
  # Install launcher
  # --------------------------------------------------------------------------
  if(NOT ${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT)
    install(TARGETS GenerateCLPLauncher RUNTIME
      DESTINATION bin COMPONENT Development
      )
  endif()
endif()

# --------------------------------------------------------------------------
# Testing
# --------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()

# --------------------------------------------------------------------------
# Set INCLUDE_DIRS variable
# --------------------------------------------------------------------------
set(${PROJECT_NAME}_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
  CACHE INTERNAL "${PROJECT_NAME} include dirs" FORCE)
