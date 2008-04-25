project(LoadableModule)

cmake_minimum_required(VERSION 2.4)

configure_file(
  ${LoadableModule_SOURCE_DIR}/LoadableModuleConfigure.h.in 
  ${LoadableModule_BINARY_DIR}/LoadableModuleConfigure.h
  )

## ITK is required for expat.h
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

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
        ${LoadableModule_BINARY_DIR}/CMake
        ${LoadableModule_SOURCE_DIR}/CMake
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

set(include_dirs
  ${LoadableModule_BINARY_DIR}
  ${LoadableModule_SOURCE_DIR}
  )

if(USE_PYTHON)
  find_package(PythonLibs)
endif(USE_PYTHON)

if(USE_PYTHON)
  # Python requires a pointer to the Slicer Application
  set(include_dirs ${include_dirs} ${PYTHON_INCLUDE_PATH})
endif(USE_PYTHON)

include_directories(${include_dirs})

GET_PERSISTENT_PROPERTY(Slicer3_INCLUDE_DIRS tmp)
SET_PERSISTENT_PROPERTY(Slicer3_INCLUDE_DIRS ${tmp} ${include_dirs})

add_library(LoadableModule ${LoadableModule_SRCS})

GET_PERSISTENT_PROPERTY(Slicer3_LIBRARIES tmp)
SET_PERSISTENT_PROPERTY(Slicer3_LIBRARIES ${tmp} LoadableModule)

if(USE_PYTHON)
  add_definitions(-DUSE_PYTHON)
endif(USE_PYTHON)

if(NOT WIN32)
  if(NOT APPLE)
    target_link_libraries(LoadableModule util)
  endif(NOT APPLE)
endif(NOT WIN32)

target_link_libraries(LoadableModule
  ITKEXPAT
  itksys
  )
if(USE_PYTHON)
  target_link_libraries(LoadableModule
    ${PYTHON_LIBRARIES}
    )
endif(USE_PYTHON)

if(NOT WIN32)
  if(HAVE_BFD)
    target_link_libraries(LoadableModule bfd iberty)
  endif(HAVE_BFD)
endif(NOT WIN32)

if(BUILD_TESTING)
  subdirs(Testing)
endif(BUILD_TESTING)

include(GenerateLoadableModuleConfig.cmake) 

configure_file(${LoadableModule_SOURCE_DIR}/UseLoadableModule.cmake.in
  ${LoadableModule_BINARY_DIR}/UseLoadableModule.cmake COPYONLY IMMEDIATE)

install(TARGETS LoadableModule 
  RUNTIME DESTINATION bin COMPONENT RuntimeLibraries
  LIBRARY DESTINATION lib COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION lib COMPONENT Development)

file(GLOB __files1 "${LoadableModule_SOURCE_DIR}/*.h")
install(FILES ${__files1} DESTINATION include/LoadableModule COMPONENT Development)
install(FILES ${LoadableModule_BINARY_DIR}/LoadableModuleConfigure.h DESTINATION include/LoadableModule COMPONENT Development)

install(FILES 
  ${LoadableModule_BINARY_DIR}/UseLoadableModule.cmake 
  ${LoadableModule_BINARY_DIR}/install/LoadableModuleConfig.cmake 
  DESTINATION lib/LoadableModule COMPONENT Development)
