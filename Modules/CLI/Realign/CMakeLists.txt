project( Realign )

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

slicerMacroBuildCLI(
  NAME ${PROJECT_NAME}
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h
  ADDITIONAL_SRCS
    vtkPrincipalAxesAlign.cxx
  TARGET_LIBRARIES ModuleDescriptionParser vtkTeem MRMLCore SlicerBaseCLI
  INCLUDE_DIRECTORIES
    ${vtkTeem_INCLUDE_DIRS}
    ${MRMLCore_INCLUDE_DIRS}
    ${vtkITK_INCLUDE_DIRS}
    ${SlicerBaseCLI_SOURCE_DIR} ${SlicerBaseCLI_BINARY_DIR}
  LINK_DIRECTORIES
    ${ModuleDescriptionParser_BINARY_DIR}
    ${vtkTeem_LIB_DIR}
  )

if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()

