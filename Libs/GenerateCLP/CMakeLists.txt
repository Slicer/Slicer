PROJECT(GenerateCLP)

CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

FIND_PACKAGE(ModuleDescriptionParser REQUIRED)
IF(ModuleDescriptionParser_FOUND)
  INCLUDE(${ModuleDescriptionParser_USE_FILE})
ENDIF(ModuleDescriptionParser_FOUND)

FIND_PACKAGE(TCLAP REQUIRED)
IF(TCLAP_FOUND)
  INCLUDE(${TCLAP_USE_FILE})
ENDIF(TCLAP_FOUND)

FIND_PACKAGE(ITK REQUIRED)
IF(ITK_FOUND)
  INCLUDE(${ITK_USE_FILE})
ENDIF(ITK_FOUND)

INCLUDE(UseGenerateCLP.cmake)

INCLUDE(GenerateGenerateCLPConfig.cmake)

# If we followed the standard pattern, UseGenerateCLP.cmake would be 
# a configured file
#
#CONFIGURE_FILE(${GenerateCLP_SOURCE_DIR}/UseGenerateCLP.cmake.in 
#               ${GenerateCLP_BINARY_DIR}/UseGenerateCLP.cmake 
#               COPYONLY IMMEDIATE)


SET (GENERATECLP_SOURCE GenerateCLP.cxx)
ADD_EXECUTABLE ( GenerateCLP ${GENERATECLP_SOURCE})
TARGET_LINK_LIBRARIES ( GenerateCLP
  ModuleDescriptionParser
  itksys
  ITKEXPAT)

IF(BUILD_TESTING)
  SUBDIRS(Testing)
ENDIF(BUILD_TESTING)

INSTALL(TARGETS GenerateCLP RUNTIME DESTINATION bin COMPONENT Development)
INSTALL(FILES ${GenerateCLP_SOURCE_DIR}/UseGenerateCLP.cmake 
              ${GenerateCLP_BINARY_DIR}/install/GenerateCLPConfig.cmake
              DESTINATION lib/GenerateCLP
              COMPONENT Development)
