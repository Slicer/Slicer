
slicerMacroBuildCLI(
  NAME DiffusionTensorMathematics
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h
  TARGET_LIBRARIES MRML vtkTeem
  LINK_DIRECTORIES
    ${MRML_LIB_DIR}
    ${vtkTeem_LIB_DIR}
  INCLUDE_DIRECTORIES
    ${MRML_SOURCE_DIR}    ${MRML_BINARY_DIR}
    ${vtkTeem_SOURCE_DIR} ${vtkTeem_BINARY_DIR}
  EXECUTABLE_ONLY
  )

if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()

