PROJECT(ModuleDescriptionParser)

CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

CONFIGURE_FILE(
  ${ModuleDescriptionParser_SOURCE_DIR}/ModuleDescriptionParserConfigure.h.in 
  ${ModuleDescriptionParser_BINARY_DIR}/ModuleDescriptionParserConfigure.h
)

CONFIGURE_FILE(
  ${ModuleDescriptionParser_SOURCE_DIR}/ModuleDescriptionParserConfig.cmake.in 
  ${ModuleDescriptionParser_BINARY_DIR}/ModuleDescriptionParserConfig.cmake 
)

## ITK is required for expat.h
FIND_PACKAGE(ITK REQUIRED)
INCLUDE(${ITK_USE_FILE})

SET(ModuleDescriptionParser_SRCS
  ModuleParameter.cxx
  ModuleParameterGroup.cxx
  ModuleDescription.cxx
  ModuleDescriptionParser.cxx
  ModuleProcessInformation.cxx
  ModuleLogo.cxx
  ModuleFactory.cxx
  BatchMakeUtilities.cxx
)

IF (USE_BFD)
  IF (NOT WIN32)
    INCLUDE(CheckIncludeFile)
    CHECK_INCLUDE_FILE(bfd.h HAVE_BFD_HEADER)

    IF (HAVE_BFD_HEADER)
       # make sure we can build with libbfd
       MESSAGE(STATUS "Testing libbfd")
       TRY_COMPILE(HAVE_BFD
                   ${ModuleDescriptionParser_BINARY_DIR}/CMake
                   ${ModuleDescriptionParser_SOURCE_DIR}/CMake
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
      SET(ModuleDescriptionParser_SRCS 
            ${ModuleDescriptionParser_SRCS} 
            BinaryFileDescriptor.cxx)
    ENDIF (HAVE_BFD)
  ENDIF (NOT WIN32)
ENDIF (USE_BFD)


INCLUDE_DIRECTORIES(
  ${ModuleDescriptionParser_SOURCE_DIR} 
  ${ModuleDescriptionParser_BINARY_DIR}
)

IF(USE_PYTHON)
  FIND_PACKAGE(PYTHONLIBS)
ENDIF(USE_PYTHON)

IF(USE_PYTHON)
# Python requires a pointer to the Slicer Application
INCLUDE_DIRECTORIES(
  ${PYTHON_INCLUDE_PATH}
)
ENDIF(USE_PYTHON)

ADD_LIBRARY(ModuleDescriptionParser ${ModuleDescriptionParser_SRCS})
IF(USE_PYTHON)
ADD_DEFINITIONS(-DUSE_PYTHON)
ENDIF(USE_PYTHON)

IF ( NOT WIN32 )
  IF(NOT APPLE)
    TARGET_LINK_LIBRARIES( ModuleDescriptionParser util)
  ENDIF(NOT APPLE)
ENDIF ( NOT WIN32 )

TARGET_LINK_LIBRARIES(ModuleDescriptionParser
  ITKEXPAT
  itksys
)
IF(USE_PYTHON)
  TARGET_LINK_LIBRARIES(ModuleDescriptionParser
    ${PYTHON_LIBRARIES}
  )
ENDIF(USE_PYTHON)

IF (NOT WIN32)
  IF (HAVE_BFD)
    TARGET_LINK_LIBRARIES(ModuleDescriptionParser bfd iberty)
  ENDIF(HAVE_BFD)
ENDIF (NOT WIN32)

IF(BUILD_TESTING)
  SUBDIRS(Testing)
ENDIF(BUILD_TESTING)

INSTALL_TARGETS(/lib ModuleDescriptionParser)
