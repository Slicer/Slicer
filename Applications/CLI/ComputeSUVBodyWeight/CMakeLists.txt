
slicerMacroBuildCLI(
  NAME ComputeSUVBodyWeight
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h
  # LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/CTSCLogo.h
  TARGET_LIBRARIES vtkITK ModuleDescriptionParser MRML SlicerBaseLogic SlicerBaseCLI
  LINK_DIRECTORIES ${vtkITK_LIB_DIR}
  INCLUDE_DIRECTORIES
    ${vtkITK_SOURCE_DIR}           ${vtkITK_BINARY_DIR}
    ${MRML_SOURCE_DIR}             ${MRML_BINARY_DIR}
    ${SlicerBaseLogic_SOURCE_DIR}  ${SlicerBaseLogic_BINARY_DIR}
  )

if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()

