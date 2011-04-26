project(ModuleDescriptionParser)

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
  if(POLICY ${policy})
    CMAKE_POLICY(SET ${policy} NEW)
  endif()
ENDFOREACH()

# --------------------------------------------------------------------------
# Prerequisites
# --------------------------------------------------------------------------
find_package(ITK REQUIRED) ## ITK is required for expat.h
include(${ITK_USE_FILE})

if(ModuleDescriptionParser_USE_PYTHON)
  find_package(PythonLibs)
endif()

# --------------------------------------------------------------------------
# Option(s)
# --------------------------------------------------------------------------
if(NOT DEFINED BUILD_SHARED_LIBS)
  option(BUILD_SHARED_LIBS "Build with shared libraries." ON)
endif()

# --------------------------------------------------------------------------
# Include dirs
# --------------------------------------------------------------------------
set(include_dirs
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR} 
  )

if(ModuleDescriptionParser_USE_PYTHON)
  set(include_dirs ${include_dirs} ${PYTHON_INCLUDE_PATH})
  if(WIN32)
    set(include_dirs ${include_dirs} ${PYTHON_INCLUDE_PATH}/../PC)
  endif()
endif()

include_directories(${include_dirs})

# --------------------------------------------------------------------------
# Configure headers
# --------------------------------------------------------------------------
set(configure_header_file ModuleDescriptionParserConfigure.h)
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/${configure_header_file}.in 
  ${CMAKE_CURRENT_BINARY_DIR}/${configure_header_file}
  )

# --------------------------------------------------------------------------
# Install headers
# --------------------------------------------------------------------------
IF(NOT DEFINED ${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT)
  SET(${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT ON)
ENDIF()
IF(NOT ${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT)
  file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
  install(
    FILES ${headers} "${CMAKE_CURRENT_BINARY_DIR}/${configure_header_file}"
    DESTINATION include/${PROJECT_NAME} COMPONENT Development
    )
ENDIF()

# --------------------------------------------------------------------------
# Sources
# --------------------------------------------------------------------------
set(ModuleDescriptionParser_SRCS
  ModuleParameter.cxx
  ModuleParameterGroup.cxx
  ModuleDescriptionUtilities.cxx
  ModuleDescription.cxx
  ModuleDescriptionParser.cxx
  ModuleProcessInformation.cxx
  ModuleLogo.cxx
  ModuleFactory.cxx
  BatchMakeUtilities.cxx
  )

# --------------------------------------------------------------------------
# Enable Binary File Descriptor support if available
# --------------------------------------------------------------------------
if(USE_BFD)
  if(NOT WIN32)
    include(CheckIncludeFile)
    check_include_file(bfd.h HAVE_BFD_HEADER)

    if(HAVE_BFD_HEADER)
      # make sure we can build with libbfd
      message(STATUS "Testing libbfd")
      try_compile(HAVE_BFD
        ${CMAKE_CURRENT_BINARY_DIR}/CMake
        ${CMAKE_CURRENT_SOURCE_DIR}/CMake
        TestBFD
        CMAKE_FLAGS 
        -DBUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        OUTPUT_VARIABLE OUTPUT)
      message(${OUTPUT})

      if(HAVE_BFD)
        message(STATUS "Testing libbfd - ok. ModuleFactory will look for global symbols in plugin executables.")
      else()
        message(STATUS "Testing libbfd - error.  ModuleFactory will not look for global symbols in plugin executables.")
      endif()

    endif()

    if(HAVE_BFD)
      set(ModuleDescriptionParser_SRCS 
        ${ModuleDescriptionParser_SRCS} 
        BinaryFileDescriptor.cxx)
    endif()
  endif()
endif()

# --------------------------------------------------------------------------
# Build library
# --------------------------------------------------------------------------
set(lib_name ModuleDescriptionParser)
add_library(${lib_name} ${ModuleDescriptionParser_SRCS})

## Always build an explicitly static library for linking against GenerateCLP so that
## GenerateCLP can be run from CMake without having to set DYLD_LIBRARY_PATH or LD_LIBRARY_PATH
## to the future location of of libModuleDescriptionParser.so
## add_library(${lib_name}-static STATIC
  ## ${ModuleDescriptionParser_SRCS}
  ## )

set(libs
  ITKEXPAT
  itksys
  )

#
# Append extra platform dependent libraries required for linking
#
if(ModuleDescriptionParser_USE_PYTHON)
  list(APPEND libs ${PYTHON_LIBRARIES})
endif()

if(NOT WIN32 AND NOT APPLE AND NOT UNIX)
  list(APPEND libs util pthread)
endif()

if(UNIX)
  list(APPEND libs pthread)
endif()

if(NOT WIN32 AND HAVE_BFD)
  list(APPEND libs bfd iberty)
endif()

target_link_libraries(${lib_name} ${libs})
## target_link_libraries(${lib_name}-static ${link_libs})

#
# Apply user-defined properties to the library target.
#
if(Slicer_LIBRARY_PROPERTIES)
  SET_TARGET_PROPERTIES(${lib_name} PROPERTIES ${Slicer_LIBRARY_PROPERTIES})
endif()


# --------------------------------------------------------------------------
# Install library
# --------------------------------------------------------------------------
if(NOT DEFINED ${PROJECT_NAME}_INSTALL_BIN_DIR)
  SET(${PROJECT_NAME}_INSTALL_BIN_DIR bin)
endif()

if(NOT DEFINED ${PROJECT_NAME}_INSTALL_LIB_DIR)
  SET(${PROJECT_NAME}_INSTALL_LIB_DIR lib/${PROJECT_NAME})
endif()

install(TARGETS ${lib_name}
  RUNTIME DESTINATION ${${PROJECT_NAME}_INSTALL_BIN_DIR} COMPONENT RuntimeLibraries
  LIBRARY DESTINATION ${${PROJECT_NAME}_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${${PROJECT_NAME}_INSTALL_LIB_DIR} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Testing
# --------------------------------------------------------------------------

if(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
endif()

# --------------------------------------------------------------------------
# Configure support files
# --------------------------------------------------------------------------
include(GenerateModuleDescriptionParserConfig.cmake) 

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/UseModuleDescriptionParser.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/UseModuleDescriptionParser.cmake COPYONLY)
  
# --------------------------------------------------------------------------
# Install support files
# --------------------------------------------------------------------------
IF(NOT ${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT)
  install(FILES 
    ${CMAKE_CURRENT_BINARY_DIR}/UseModuleDescriptionParser.cmake 
    ${CMAKE_CURRENT_BINARY_DIR}/install/ModuleDescriptionParserConfig.cmake 
    DESTINATION lib/${lib_name} COMPONENT Development)
ENDIF()

