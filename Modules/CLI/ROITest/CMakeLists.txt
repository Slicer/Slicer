
#-----------------------------------------------------------------------------
set(MODULE_NAME CLIROITest)

#-----------------------------------------------------------------------------

#
# SlicerExecutionModel
#
find_package(SlicerExecutionModel REQUIRED)
include(${SlicerExecutionModel_USE_FILE})

#-----------------------------------------------------------------------------
SEMMacroBuildCLI(
  NAME ${MODULE_NAME}
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h
  TARGET_LIBRARIES
    # ${VTK_LIBRARIES} # Not needed: All dependencies are transitively satisfied by other targets
    ModuleDescriptionParser ${ITK_LIBRARIES}
    vtkTeem
    MRMLCore
    SlicerBaseCLI
  INCLUDE_DIRECTORIES
    ${vtkTeem_INCLUDE_DIRS}
    ${MRMLCore_INCLUDE_DIRS}
    ${vtkITK_INCLUDE_DIRS}
    ${SlicerBaseCLI_SOURCE_DIR} ${SlicerBaseCLI_BINARY_DIR}
  NO_INSTALL
  )

