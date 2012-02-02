
slicerMacroBuildCLI(
  NAME DWIToDTIEstimation
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h
  TARGET_LIBRARIES MRMLCore vtkTeem SlicerBaseLogic
  LINK_DIRECTORIES
    ${vtkTeem_LIB_DIR}
  INCLUDE_DIRECTORIES
    ${vtkITK_INCLUDE_DIRS}
    ${MRMLCore_INCLUDE_DIRS}
    ${vtkTeem_INCLUDE_DIRS}
    ${SlicerBaseLogic_SOURCE_DIR}
    ${SlicerBaseLogic_BINARY_DIR}
  EXECUTABLE_ONLY
  )

if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()

