PROJECT(GenerateCLP)

CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

IF(Slicer3_SOURCE_DIR)
  SET(ModuleDescriptionParser_DIR ${Slicer3_BINARY_DIR}/Libs/ModuleDescriptionParser)
ENDIF(Slicer3_SOURCE_DIR)

FIND_PACKAGE(ModuleDescriptionParser REQUIRED)
FIND_PACKAGE(ITK REQUIRED)

LINK_DIRECTORIES(${ModuleDescriptionParser_BINARY_DIR})

INCLUDE(${ITK_USE_FILE})
INCLUDE(UseGenerateCLP.cmake)

CONFIGURE_FILE(
  ${GenerateCLP_SOURCE_DIR}/GenerateCLPConfig.cmake.in 
  ${GenerateCLP_BINARY_DIR}/GenerateCLPConfig.cmake 
)

SET (GENERATECLP_SOURCE GenerateCLP.cxx)
LINK_DIRECTORIES(${ModuleDescriptionParser_LIB_DIR})
ADD_EXECUTABLE ( GenerateCLP ${GENERATECLP_SOURCE})
TARGET_LINK_LIBRARIES ( GenerateCLP
  ModuleDescriptionParser
  ITKEXPAT)

SUBDIRS(Testing)

