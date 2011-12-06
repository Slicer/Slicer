
slicerMacroBuildCLI(
  NAME ComputeSUVBodyWeight
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h
  # LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/CTSCLogo.h
  TARGET_LIBRARIES vtkITK ModuleDescriptionParser MRMLCore SlicerBaseLogic SlicerBaseCLI ITKIO
  INCLUDE_DIRECTORIES
    ${vtkITK_INCLUDE_DIRS}
    ${MRMLCore_INCLUDE_DIRS}
    ${SlicerBaseLogic_SOURCE_DIR}  ${SlicerBaseLogic_BINARY_DIR}
  EXECUTABLE_ONLY
  )

if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()

