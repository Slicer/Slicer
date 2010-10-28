project(LoadableModule)

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

## ITK is required for expat.h

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

if(LoadableModule_USE_PYTHON)
  find_package(PythonLibs)
endif(LoadableModule_USE_PYTHON)

if(NOT DEFINED BUILD_SHARED_LIBS)
  option(BUILD_SHARED_LIBS "Build with shared libraries." ON)
endif(NOT DEFINED BUILD_SHARED_LIBS)
 
# --------------------------------------------------------------------------
# Include dirs

set(include_dirs
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  )

if(LoadableModule_USE_PYTHON)
  # Python requires a pointer to the Slicer Application
  set(include_dirs ${include_dirs} ${PYTHON_INCLUDE_PATH})
  if(WIN32)
    set(include_dirs ${include_dirs} ${PYTHON_INCLUDE_PATH}/../PC)
  endif(WIN32)
endif(LoadableModule_USE_PYTHON)

include_directories(${include_dirs})

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/LoadableModuleConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/LoadableModuleConfigure.h
  )

file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
install(FILES 
  ${headers} 
  "${CMAKE_CURRENT_BINARY_DIR}/LoadableModuleConfigure.h"
  DESTINATION include/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Sources

set(LoadableModule_SRCS
  LoadableModuleDescription.cxx
  LoadableModuleDescriptionParser.cxx
  LoadableModuleFactory.cxx
  )

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
      else(HAVE_BFD)
        message(STATUS "Testing libbfd - error.  ModuleFactory will not look for global symbols in plugin executables.")
      endif(HAVE_BFD)
    endif(HAVE_BFD_HEADER)
    
    if(HAVE_BFD)
      set(LoadableModule_SRCS 
        ${LoadableModule_SRCS} 
        BinaryFileDescriptor.cxx)
    endif(HAVE_BFD)
  endif(NOT WIN32)
endif(USE_BFD)

# --------------------------------------------------------------------------
# Build and install the library

set(lib_name LoadableModule)
add_library(${lib_name} 
  ${LoadableModule_SRCS}
  )

set(link_libs
  ITKEXPAT
  itksys
  )
if(LoadableModule_USE_PYTHON)
  set(link_libs ${link_libs} ${PYTHON_LIBRARIES})
endif(LoadableModule_USE_PYTHON)
if(NOT WIN32 AND NOT APPLE AND NOT UNIX)
  set(link_libs ${link_libs} util)
endif(NOT WIN32 AND NOT APPLE AND NOT UNIX)
if(NOT WIN32 AND HAVE_BFD)
  set(link_libs ${link_libs} bfd iberty)
endif(NOT WIN32 AND HAVE_BFD)

target_link_libraries(${lib_name} ${link_libs})

# Apply user-defined properties to the library target.
IF(Slicer_LIBRARY_PROPERTIES)
  SET_TARGET_PROPERTIES(${lib_name} PROPERTIES
     ${Slicer_LIBRARY_PROPERTIES}
  )
ENDIF(Slicer_LIBRARY_PROPERTIES)

install(TARGETS ${lib_name} 
  RUNTIME DESTINATION bin COMPONENT RuntimeLibraries
  LIBRARY DESTINATION lib/${PROJECT_NAME} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION lib/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Testing

if(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
endif(BUILD_TESTING)

# --------------------------------------------------------------------------
# Install support files

include(GenerateLoadableModuleConfig.cmake) 

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/UseLoadableModule.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/UseLoadableModule.cmake COPYONLY)

install(FILES 
  ${CMAKE_CURRENT_BINARY_DIR}/UseLoadableModule.cmake 
  ${CMAKE_CURRENT_BINARY_DIR}/install/LoadableModuleConfig.cmake 
  DESTINATION lib/${PROJECT_NAME} COMPONENT Development
  )
