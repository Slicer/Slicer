
slicerMacroBuildCLI(
  NAME ProbeVolumeWithModel
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h
  TARGET_LIBRARIES MRML vtkTeem SlicerBaseLogic
  INCLUDE_DIRECTORIES
    ${vtkITK_SOURCE_DIR}  ${vtkITK_BINARY_DIR}
    ${MRML_SOURCE_DIR}    ${MRML_BINARY_DIR}
    ${vtkTeem_SOURCE_DIR} ${vtkTeem_BINARY_DIR}
  EXECUTABLE_ONLY
  )

if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()

