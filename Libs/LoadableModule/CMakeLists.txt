PROJECT(LoadableModule)

CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

CONFIGURE_FILE(
  ${LoadableModule_SOURCE_DIR}/LoadableModuleConfigure.h.in 
  ${LoadableModule_BINARY_DIR}/LoadableModuleConfigure.h
)

## ITK is required for expat.h
FIND_PACKAGE(ITK REQUIRED)
INCLUDE(${ITK_USE_FILE})

SET(LoadableModule_SRCS
  LoadableModuleDescription.cxx
  LoadableModuleDescriptionParser.cxx
  LoadableModuleFactory.cxx
)

IF (USE_BFD)
  IF (NOT WIN32)
    INCLUDE(CheckIncludeFile)
    CHECK_INCLUDE_FILE(bfd.h HAVE_BFD_HEADER)

    IF (HAVE_BFD_HEADER)
       # make sure we can build with libbfd
       MESSAGE(STATUS "Testing libbfd")
       TRY_COMPILE(HAVE_BFD
                   ${LoadableModule_BINARY_DIR}/CMake
                   ${LoadableModule_SOURCE_DIR}/CMake
                   TestBFD
                   CMAKE_FLAGS 
                      -DBUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}
                      -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                   OUTPUT_VARIABLE OUTPUT)
       MESSAGE(${OUTPUT})
       IF (HAVE_BFD)
         MESSAGE(STATUS "Testing libbfd - ok. ModuleFactory will look for global symbols in plugin executables.")
       ELSE (HAVE_BFD)
         MESSAGE(STATUS "Testing libbfd - error.  ModuleFactory will not look for global symbols in plugin executables.")
       ENDIF (HAVE_BFD)
    ENDIF (HAVE_BFD_HEADER)

    IF (HAVE_BFD)
      SET(LoadableModule_SRCS 
            ${LoadableModule_SRCS} 
            BinaryFileDescriptor.cxx)
    ENDIF (HAVE_BFD)
  ENDIF (NOT WIN32)
ENDIF (USE_BFD)


INCLUDE_DIRECTORIES(
  ${LoadableModule_BINARY_DIR}
  ${LoadableModule_SOURCE_DIR}
  ${SlicerBaseLogic_SOURCE_DIR}
  ${SlicerBaseGUI_SOURCE_DIR}
)

IF(USE_PYTHON)
  FIND_PACKAGE(PythonLibs)
ENDIF(USE_PYTHON)

IF(USE_PYTHON)
# Python requires a pointer to the Slicer Application
INCLUDE_DIRECTORIES(
  ${PYTHON_INCLUDE_PATH}
)
ENDIF(USE_PYTHON)

ADD_LIBRARY(LoadableModule ${LoadableModule_SRCS})
IF(USE_PYTHON)
ADD_DEFINITIONS(-DUSE_PYTHON)
ENDIF(USE_PYTHON)

IF ( NOT WIN32 )
  IF(NOT APPLE)
    TARGET_LINK_LIBRARIES( LoadableModule util)
  ENDIF(NOT APPLE)
ENDIF ( NOT WIN32 )

TARGET_LINK_LIBRARIES(LoadableModule
  ITKEXPAT
  itksys
)
IF(USE_PYTHON)
   TARGET_LINK_LIBRARIES(LoadableModule
     ${PYTHON_LIBRARIES}
  )
ENDIF(USE_PYTHON)

IF (NOT WIN32)
  IF (HAVE_BFD)
    TARGET_LINK_LIBRARIES(LoadableModule bfd iberty)
  ENDIF(HAVE_BFD)
ENDIF (NOT WIN32)

IF(BUILD_TESTING)
  SUBDIRS(Testing)
ENDIF(BUILD_TESTING)


INCLUDE (GenerateLoadableModuleConfig.cmake) 

CONFIGURE_FILE(${LoadableModule_SOURCE_DIR}/UseLoadableModule.cmake.in
               ${LoadableModule_BINARY_DIR}/UseLoadableModule.cmake COPYONLY IMMEDIATE)


INSTALL(TARGETS LoadableModule 
    RUNTIME DESTINATION bin COMPONENT RuntimeLibraries
    LIBRARY DESTINATION lib COMPONENT RuntimeLibraries
    ARCHIVE DESTINATION lib COMPONENT Development)

FILE(GLOB __files1 "${LoadableModule_SOURCE_DIR}/*.h")
INSTALL(FILES ${__files1} DESTINATION include/LoadableModule COMPONENT Development)
INSTALL(FILES ${LoadableModule_BINARY_DIR}/LoadableModuleConfigure.h DESTINATION include/LoadableModule COMPONENT Development)

INSTALL(FILES 
     ${LoadableModule_BINARY_DIR}/UseLoadableModule.cmake 
     ${LoadableModule_BINARY_DIR}/install/LoadableModuleConfig.cmake 
     DESTINATION lib/LoadableModule COMPONENT Development)
