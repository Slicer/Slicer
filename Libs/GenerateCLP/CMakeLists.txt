PROJECT( GenerateCLP )

FIND_PACKAGE ( ITK )
IF ( ITK_FOUND )
  INCLUDE(${ITK_USE_FILE})
ELSE ( ITK_FOUND )
  MESSAGE ( FATAL_ERROR "Cannot build without ITK" )
ENDIF ( ITK_FOUND )

IF (Slicer3_SOURCE_DIR)
  SET(ModuleDescriptionParser_LIB_DIR)
ELSE (Slicer3_SOURCE_DIR)
  FIND_PACKAGE(ModuleDescriptionParser REQUIRED)
  SET(ModuleDescriptionParser_LIB_DIR ${ModuleDescriptionParser_BINARY_DIR}/)
ENDIF (Slicer3_SOURCE_DIR)

INCLUDE(UseGenerateCLP.cmake)

SET (GENERATECLP_SOURCE GenerateCLP.cxx)
ADD_EXECUTABLE ( GenerateCLP ${GENERATECLP_SOURCE})
TARGET_LINK_LIBRARIES ( GenerateCLP
  ${ModuleDescriptionParser_LIB_DIR}ModuleDescriptionParser
  ITKEXPAT)

SUBDIRS(Testing)

