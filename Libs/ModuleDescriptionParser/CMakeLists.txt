PROJECT(ModuleDescriptionParser)

CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

## ITK is required
FIND_PACKAGE(ITK REQUIRED)
IF (NOT ITK_FOUND)
  MESSAGE (FATAL_ERROR "Need ITK to build")
ENDIF (NOT ITK_FOUND)

INCLUDE(${ITK_USE_FILE})

SET(ModuleDescriptionParser_SRCS
  ModuleParameter.cxx
  ModuleParameterGroup.cxx
  ModuleDescription.cxx
  ModuleDescriptionParser.cxx
  ModuleProcessInformation.cxx
)

INCLUDE_DIRECTORIES(
  ${ModuleDescriptionParser_SOURCE_DIR} 
  ${ModuleDescriptionParser_BINARY_DIR}
)

ADD_LIBRARY(ModuleDescriptionParser ${ModuleDescriptionParser_SRCS})

TARGET_LINK_LIBRARIES(ModuleDescriptionParser ITKEXPAT)

CONFIGURE_FILE(
  ${ModuleDescriptionParser_SOURCE_DIR}/ModuleDescriptionParserConfigure.h.in 
  ${ModuleDescriptionParser_BINARY_DIR}/ModuleDescriptionParserConfigure.h
)

CONFIGURE_FILE(
  ${ModuleDescriptionParser_SOURCE_DIR}/ModuleDescriptionParserConfig.cmake.in 
  ${ModuleDescriptionParser_BINARY_DIR}/ModuleDescriptionParserConfig.cmake 
)

SUBDIRS(Testing)
INSTALL_TARGETS(/lib ModuleDescriptionParser)
