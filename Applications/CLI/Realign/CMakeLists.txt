project( Realign )

find_package(GenerateCLP REQUIRED)
include(${GenerateCLP_USE_FILE})

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

slicerMacroBuildCLI(
  NAME ${PROJECT_NAME}
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h
  ADDITIONAL_SRCS 
    vtkPrincipalAxesAlign.cxx
  TARGET_LIBRARIES ModuleDescriptionParser vtkTeem MRML SlicerBaseCLI
  INCLUDE_DIRECTORIES 
    ${vtkTeem_SOURCE_DIR}       ${vtkTeem_BINARY_DIR}
    ${MRML_SOURCE_DIR}          ${MRML_BINARY_DIR}
    ${SlicerBaseCLI_SOURCE_DIR} ${SlicerBaseCLI_BINARY_DIR}
  LINK_DIRECTORIES
    ${ModuleDescriptionParser_BINARY_DIR} 
    ${vtkTeem_LIB_DIR}
  )

