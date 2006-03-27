PROJECT(GradientAnisotropicDiffusionFilter)

# --------------------------------------------------------------------------
# Sources

SET(GradientAnisotropicDiffusionFilter_SRCS
  vtkGradientAnisotropicDiffusionFilterModule.cxx
  vtkGradientAnisotropicDiffusionFilterWidget.cxx
  vtkMRMLGradientAnisotropicDiffusionFilterNode.cxx
)

# Abstract/pure virtual classes

#SET_SOURCE_FILES_PROPERTIES(
  #vtkModule.cxx
  #PROPERTIES
  #ABSTRACT "TRUE"
  #)

# Helper classes

#SET_SOURCE_FILES_PROPERTIES(
  #vtkModule.cxx
  #PROPERTIES
  #WRAP_EXCLUDE "TRUE"
  #)

# --------------------------------------------------------------------------
# Include dirs

INCLUDE_DIRECTORIES(
  ${ModuleBaseClasses_SOURCE_DIR}
  ${ModuleBaseClasses_BINARY_DIR}
  ${GradientAnisotropicDiffusionFilter_SOURCE_DIR}
  ${GradientAnisotropicDiffusionFilter_BINARY_DIR}
  ${SlicerBase_SOURCE_DIR}
  ${SlicerBase_BINARY_DIR}
  ${SlicerBaseLogic_SOURCE_DIR}
  ${SlicerBaseLogic_BINARY_DIR}
  ${SlicerBaseGUI_SOURCE_DIR}
  ${SlicerBaseGUI_BINARY_DIR}
  ${MRML_SOURCE_DIR}
  ${MRML_BINARY_DIR}
  ${VTK_INCLUDE_DIR}
  )




# --------------------------------------------------------------------------
# Build the library

ADD_LIBRARY(GradientAnisotropicDiffusionFilter
  ${GradientAnisotropicDiffusionFilter_SRCS}
  )

TARGET_LINK_LIBRARIES(GradientAnisotropicDiffusionFilter
  SlicerBaseLogic 
  SlicerBaseGUI 
  MRML
  vtkITK
  ITKCommon 
  ITKIO
  ${KWWidgets_LIBRARIES} )



# --------------------------------------------------------------------------
# Testing (requires some of the examples)

IF(BUILD_TESTING)
  SUBDIRS(Testing)
ENDIF(BUILD_TESTING)

CONFIGURE_FILE(
  ${GradientAnisotropicDiffusionFilter_SOURCE_DIR}/vtkGradientAnisotropicDiffusionFilterConfigure.h.in 
  ${GradientAnisotropicDiffusionFilter_BINARY_DIR}/vtkModulesBaseClassesConfigure.h
)
