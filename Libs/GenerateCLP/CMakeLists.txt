PROJECT(GenerateCLP)

CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

FIND_PACKAGE(ModuleDescriptionParser NO_MODULE REQUIRED)
IF(ModuleDescriptionParser_FOUND)
  INCLUDE(${ModuleDescriptionParser_USE_FILE})
ENDIF(ModuleDescriptionParser_FOUND)

FIND_PACKAGE(TCLAP NO_MODULE REQUIRED)
IF(TCLAP_FOUND)
  INCLUDE(${TCLAP_USE_FILE})
ENDIF(TCLAP_FOUND)

FIND_PACKAGE(ITK REQUIRED)
IF(ITK_FOUND)
  INCLUDE(${ITK_USE_FILE})
ENDIF(ITK_FOUND)

SET (GENERATECLP_SOURCE GenerateCLP.cxx)
ADD_EXECUTABLE ( GenerateCLP ${GENERATECLP_SOURCE})
GET_TARGET_PROPERTY( GenerateCLP_EXE_PATH GenerateCLP LOCATION )

TARGET_LINK_LIBRARIES ( GenerateCLP
  ModuleDescriptionParser
  itksys
  ITKEXPAT)

IF(BUILD_TESTING)
  SUBDIRS(Testing)
ENDIF(BUILD_TESTING)


INCLUDE(GenerateGenerateCLPConfig.cmake)

INSTALL(TARGETS GenerateCLP RUNTIME DESTINATION bin COMPONENT Development)
INSTALL(FILES ${GenerateCLP_BINARY_DIR}/GenerateCLPConfig.cmake_install
              DESTINATION lib/GenerateCLP
              COMPONENT Development
              RENAME GenerateCLPConfig.cmake)
INSTALL(FILES ${GenerateCLP_BINARY_DIR}/UseGenerateCLP.cmake_install
              DESTINATION lib/GenerateCLP
              COMPONENT Development
              RENAME UseGenerateCLP.cmake)
