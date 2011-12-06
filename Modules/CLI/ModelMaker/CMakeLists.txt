
# TODO: Remove SlicerBaseLogic once pass in color node information
slicerMacroBuildCLI(
  NAME ModelMaker
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h
  TARGET_LIBRARIES vtkITK ModuleDescriptionParser MRMLCore SlicerBaseLogic SlicerBaseCLI
  INCLUDE_DIRECTORIES
    ${vtkITK_INCLUDE_DIRS}
    ${MRMLCore_INCLUDE_DIRS}
    ${SlicerBaseLogic_SOURCE_DIR} ${SlicerBaseLogic_BINARY_DIR}
  )

if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()

