project(LoadableModule)

cmake_minimum_required(VERSION 2.4)
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

## ITK is required for expat.h

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

if(Slicer3_USE_PYTHON)
  find_package(PythonLibs)
endif(Slicer3_USE_PYTHON)

# --------------------------------------------------------------------------
# Include dirs

set(include_dirs
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  )

if(Slicer3_USE_PYTHON)
  # Python requires a pointer to the Slicer Application
  set(include_dirs ${include_dirs} ${PYTHON_INCLUDE_PATH})
endif(Slicer3_USE_PYTHON)

include_directories(${include_dirs})

slicer3_get_persistent_property(Slicer3_Libs_INCLUDE_DIRS tmp)
slicer3_set_persistent_property(Slicer3_Libs_INCLUDE_DIRS ${tmp} ${include_dirs})

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
if(Slicer3_USE_PYTHON)
  set(link_libs ${link_libs} ${PYTHON_LIBRARIES})
endif(Slicer3_USE_PYTHON)
if(NOT WIN32 AND NOT APPLE)
  set(link_libs ${link_libs} util)
endif(NOT WIN32 AND NOT APPLE)
if(NOT WIN32 AND HAVE_BFD)
  set(link_libs ${link_libs} bfd iberty)
endif(NOT WIN32 AND HAVE_BFD)

target_link_libraries(${lib_name} ${link_libs})

slicer3_get_persistent_property(Slicer3_Libs_LIBRARIES tmp)
slicer3_set_persistent_property(Slicer3_Libs_LIBRARIES ${tmp} ${lib_name})

install(TARGETS ${lib_name} 
  RUNTIME DESTINATION bin COMPONENT RuntimeLibraries
  LIBRARY DESTINATION lib/${PROJECT_NAME} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION lib/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Testing

if(BUILD_TESTING)
  subdirs(Testing)
endif(BUILD_TESTING)

# --------------------------------------------------------------------------
# Install support files

include(GenerateLoadableModuleConfig.cmake) 

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/UseLoadableModule.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/UseLoadableModule.cmake COPYONLY IMMEDIATE)

install(FILES 
  ${CMAKE_CURRENT_BINARY_DIR}/UseLoadableModule.cmake 
  ${CMAKE_CURRENT_BINARY_DIR}/install/LoadableModuleConfig.cmake 
  DESTINATION lib/${PROJECT_NAME} COMPONENT Development
  )
