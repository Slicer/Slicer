
slicerMacroBuildCLI(
  NAME DiffusionWeightedMasking
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h
  TARGET_LIBRARIES MRML vtkTeem SlicerBaseLogic
  LINK_DIRECTORIES
    ${MRML_LIB_DIR}
    ${vtkTeem_LIB_DIR}
  INCLUDE_DIRECTORIES
    ${vtkITK_INCLUDE_DIRS}
    ${MRML_INCLUDE_DIRS}
    ${vtkTeem_INCLUDE_DIRS}
    ${SlicerBaseLogic_SOURCE_DIR}
    ${SlicerBaseLogic_BINARY_DIR}
  EXECUTABLE_ONLY
  )

