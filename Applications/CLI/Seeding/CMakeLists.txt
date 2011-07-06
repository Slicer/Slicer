
slicerMacroBuildCLI(
  NAME Seeding
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h
  TARGET_LIBRARIES MRML vtkTeem SlicerBaseLogic
  LINK_DIRECTORIES
    ${MRML_LIB_DIR}
    ${vtkTeem_LIB_DIR}
    ${SlicerBaseLogic_SOURCE_DIR}
    ${SlicerBaseLogic_BINARY_DIR}
  INCLUDE_DIRECTORIES
    ${vtkITK_SOURCE_DIR}
    ${vtkITK_BINARY_DIR}
    ${MRML_SOURCE_DIR}
    ${MRML_BINARY_DIR}
    ${vtkTeem_SOURCE_DIR}
    ${vtkTeem_BINARY_DIR}
  EXECUTABLE_ONLY
  )

