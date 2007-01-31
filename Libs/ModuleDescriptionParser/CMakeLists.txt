PROJECT(ModuleDescriptionParser)

CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

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
)
IF (NOT WIN32)
  SET(ModuleDescriptionParser_SRCS 
        ${ModuleDescriptionParser_SRCS} 
        BinaryFileDescriptor.cxx
  )
ENDIF (NOT WIN32)

INCLUDE_DIRECTORIES(
  ${ModuleDescriptionParser_SOURCE_DIR} 
  ${ModuleDescriptionParser_BINARY_DIR}
)

ADD_LIBRARY(ModuleDescriptionParser ${ModuleDescriptionParser_SRCS})

TARGET_LINK_LIBRARIES(ModuleDescriptionParser ITKEXPAT itksys)
IF (NOT WIN32)
TARGET_LINK_LIBRARIES(ModuleDescriptionParser bfd iberty)
ENDIF (NOT WIN32)

CONFIGURE_FILE(
  ${ModuleDescriptionParser_SOURCE_DIR}/ModuleDescriptionParserConfigure.h.in 
  ${ModuleDescriptionParser_BINARY_DIR}/ModuleDescriptionParserConfigure.h
)

CONFIGURE_FILE(
  ${ModuleDescriptionParser_SOURCE_DIR}/ModuleDescriptionParserConfig.cmake.in 
  ${ModuleDescriptionParser_BINARY_DIR}/ModuleDescriptionParserConfig.cmake 
)

IF(BUILD_TESTING)
  SUBDIRS(Testing)
ENDIF(BUILD_TESTING)

INSTALL_TARGETS(/lib ModuleDescriptionParser)
