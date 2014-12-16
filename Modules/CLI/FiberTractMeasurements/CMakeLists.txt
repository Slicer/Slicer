
#-----------------------------------------------------------------------------
set(MODULE_NAME FiberTractMeasurements)

# Avoid vtkNRRDReader/vtkNrrdReader filename confusion on case insensitive file systems
include_directories(BEFORE ${vtkTeem_INCLUDE_DIRS})

#-----------------------------------------------------------------------------

#
# SlicerExecutionModel
#
find_package(SlicerExecutionModel REQUIRED)
include(${SlicerExecutionModel_USE_FILE})

#-----------------------------------------------------------------------------
set(${MODULE_NAME}_TARGET_LIBRARIES
  vtkITK ModuleDescriptionParser MRMLCore SlicerBaseLogic SlicerBaseCLI
  vtkTeem ${VTK_LIBRARIES} vtkSlicerTractographyDisplayModuleMRML MRMLCLI
  )

#-----------------------------------------------------------------------------
SEMMacroBuildCLI(
  NAME ${MODULE_NAME}
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h
  TARGET_LIBRARIES ${${MODULE_NAME}_TARGET_LIBRARIES}
  INCLUDE_DIRECTORIES
    ${vtkTeem_INCLUDE_DIRS}
    ${MRMLCore_INCLUDE_DIRS}
    ${MRMLCLI_INCLUDE_DIRS}
    ${VTK_INCLUDE_DIRS}
    ${SlicerBaseLogic_SOURCE_DIR} ${SlicerBaseLogic_BINARY_DIR}
    ${vtkSlicerTractographyDisplayModuleMRML_SOURCE_DIR} ${vtkSlicerTractographyDisplayModuleMRML_BINARY_DIR}
  EXECUTABLE_ONLY
  )

#-----------------------------------------------------------------------------
if(BUILD_TESTING)
#  add_subdirectory(Testing)
endif()
