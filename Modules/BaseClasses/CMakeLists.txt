PROJECT(ModulesBaseClasses)

# --------------------------------------------------------------------------
# Sources

SET(ModulesBaseClasses_SRCS
  vtkModule.cxx 
  ModuleFactory.cxx
)

# Abstract/pure virtual classes

SET_SOURCE_FILES_PROPERTIES(
  vtkModule.cxx
  PROPERTIES
  ABSTRACT "TRUE"
  )

# Helper classes

SET_SOURCE_FILES_PROPERTIES(
  vtkModule.cxx
  PROPERTIES
  WRAP_EXCLUDE "TRUE"
  )

# --------------------------------------------------------------------------
# Include dirs

INCLUDE_DIRECTORIES(
  ${ModulesBaseClasses_SOURCE_DIR}
  ${ModulesBaseClasses_BINARY_DIR}
  ${MRML_SOURCE_DIR}
  ${MRML_BINARY_DIR}
  ${ModuleDescriptionParser_SOURCE_DIR}
  ${ModuleDescriptionParser_BINARY_DIR}
  ${SlicerBaseGUI_SOURCE_DIR}
  ${SlicerBaseGUI_BINARY_DIR}
  ${SlicerBaseLogic_SOURCE_DIR}
  ${SlicerBaseLogic_BINARY_DIR}
  ${VTK_INCLUDE_DIR}
  )




# --------------------------------------------------------------------------
# Build the library

ADD_LIBRARY(ModulesBaseClasses 
  ${ModulesBaseClasses_SRCS}
  )

IF(BUILD_SHARED_LIBS)
  INSTALL_TARGETS(${SLICER3_LIB_INSTALL_DIR} ModulesBaseClasses)
ENDIF(BUILD_SHARED_LIBS)

TARGET_LINK_LIBRARIES(ModulesBaseClasses 
  MRML
  vtkCommonTCL
  ModuleDescriptionParser
  SlicerBaseGUI
  SlicerBaseLogic
  )


# --------------------------------------------------------------------------
# Testing (requires some of the examples)

IF(BUILD_TESTING)
  SUBDIRS(Testing)
ENDIF(BUILD_TESTING)

CONFIGURE_FILE(
  ${ModulesBaseClasses_SOURCE_DIR}/vtkModulesBaseClassesConfigure.h.in 
  ${ModulesBaseClasses_BINARY_DIR}/vtkModulesBaseClassesConfigure.h
)
