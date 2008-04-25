project(ModuleDescriptionParser)

cmake_minimum_required(VERSION 2.4)

configure_file(
  ${ModuleDescriptionParser_SOURCE_DIR}/ModuleDescriptionParserConfigure.h.in 
  ${ModuleDescriptionParser_BINARY_DIR}/ModuleDescriptionParserConfigure.h
  )

## ITK is required for expat.h
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

set(ModuleDescriptionParser_SRCS
  ModuleParameter.cxx
  ModuleParameterGroup.cxx
  ModuleDescription.cxx
  ModuleDescriptionParser.cxx
  ModuleProcessInformation.cxx
  ModuleLogo.cxx
  ModuleFactory.cxx
  BatchMakeUtilities.cxx
  )

if(USE_BFD)
  if(NOT WIN32)
    include(CheckIncludeFile)
    check_include_file(bfd.h HAVE_BFD_HEADER)

    if(HAVE_BFD_HEADER)
      # make sure we can build with libbfd
      message(STATUS "Testing libbfd")
      try_compile(HAVE_BFD
        ${ModuleDescriptionParser_BINARY_DIR}/CMake
        ${ModuleDescriptionParser_SOURCE_DIR}/CMake
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
      set(ModuleDescriptionParser_SRCS 
        ${ModuleDescriptionParser_SRCS} 
        BinaryFileDescriptor.cxx)
    endif(HAVE_BFD)
  endif(NOT WIN32)
endif(USE_BFD)

set(include_dirs
  ${ModuleDescriptionParser_BINARY_DIR}
  ${ModuleDescriptionParser_SOURCE_DIR} 
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

add_library(ModuleDescriptionParser ${ModuleDescriptionParser_SRCS})

GET_PERSISTENT_PROPERTY(Slicer3_LIBRARIES tmp)
SET_PERSISTENT_PROPERTY(Slicer3_LIBRARIES ${tmp} ModuleDescriptionParser)

if(USE_PYTHON)
  add_definitions(-DUSE_PYTHON)
endif(USE_PYTHON)

if(NOT WIN32)
  if(NOT APPLE)
    target_link_libraries(ModuleDescriptionParser util)
  endif(NOT APPLE)
endif(NOT WIN32)

target_link_libraries(ModuleDescriptionParser
  ITKEXPAT
  itksys
  )
if(USE_PYTHON)
  target_link_libraries(ModuleDescriptionParser
    ${PYTHON_LIBRARIES}
    )
endif(USE_PYTHON)

if(NOT WIN32)
  if(HAVE_BFD)
    target_link_libraries(ModuleDescriptionParser bfd iberty)
  endif(HAVE_BFD)
endif(NOT WIN32)

if(BUILD_TESTING)
  subdirs(Testing)
endif(BUILD_TESTING)

include(GenerateModuleDescriptionParserConfig.cmake) 

configure_file(${ModuleDescriptionParser_SOURCE_DIR}/UseModuleDescriptionParser.cmake.in
  ${ModuleDescriptionParser_BINARY_DIR}/UseModuleDescriptionParser.cmake COPYONLY IMMEDIATE)


install(TARGETS ModuleDescriptionParser 
  RUNTIME DESTINATION bin COMPONENT RuntimeLibraries
  LIBRARY DESTINATION lib/ModuleDescriptionParser COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION lib/ModuleDescriptionParser COMPONENT Development)

file(GLOB __files1 "${ModuleDescriptionParser_SOURCE_DIR}/*.h")
install(FILES ${__files1} DESTINATION include/ModuleDescriptionParser COMPONENT Development)
install(FILES ${ModuleDescriptionParser_BINARY_DIR}/ModuleDescriptionParserConfigure.h DESTINATION include/ModuleDescriptionParser COMPONENT Development)

install(FILES 
  ${ModuleDescriptionParser_BINARY_DIR}/UseModuleDescriptionParser.cmake 
  ${ModuleDescriptionParser_BINARY_DIR}/install/ModuleDescriptionParserConfig.cmake 
  DESTINATION lib/ModuleDescriptionParser COMPONENT Development)
