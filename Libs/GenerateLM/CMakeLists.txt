PROJECT(GenerateLM)

CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

FIND_PACKAGE(GenerateCLP REQUIRED)
IF(GenerateCLP_FOUND)
  INCLUDE(${GenerateCLP_USE_FILE})
ENDIF(GenerateCLP_FOUND)

FIND_PACKAGE(LoadableModule REQUIRED)
IF(LoadableModule_FOUND)
  INCLUDE(${LoadableModule_USE_FILE})
ENDIF(LoadableModule_FOUND)

FIND_PACKAGE(TCLAP REQUIRED)
IF(TCLAP_FOUND)
  INCLUDE(${TCLAP_USE_FILE})
ENDIF(TCLAP_FOUND)

FIND_PACKAGE(ITK REQUIRED)
IF(ITK_FOUND)
  INCLUDE(${ITK_USE_FILE})
ENDIF(ITK_FOUND)

INCLUDE(UseGenerateLM.cmake)

INCLUDE(GenerateGenerateLMConfig.cmake)

# If we followed the standard pattern, UseGenerateLM.cmake would be 
# a configured file
#
#CONFIGURE_FILE(${GenerateLM_SOURCE_DIR}/UseGenerateLM.cmake.in 
#               ${GenerateLM_BINARY_DIR}/UseGenerateLM.cmake 
#               COPYONLY IMMEDIATE)


SET (GENERATELM_SOURCE GenerateLM.cxx)
GENERATECLP(GENERATELM_SOURCE GenerateLM.xml)
ADD_EXECUTABLE ( GenerateLM ${GENERATELM_SOURCE})
TARGET_LINK_LIBRARIES ( GenerateLM
  LoadableModule
  itksys
  ITKEXPAT)


INSTALL(TARGETS GenerateLM RUNTIME DESTINATION bin COMPONENT Development)
INSTALL(FILES ${GenerateLM_SOURCE_DIR}/UseGenerateLM.cmake 
              ${GenerateLM_BINARY_DIR}/install/GenerateLMConfig.cmake
              DESTINATION lib/GenerateLM
              COMPONENT Development)
