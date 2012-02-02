configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkRegisterImagesModuleMultiResApplicationsConfigure.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/vtkRegisterImagesModuleMultiResApplicationsConfigure.h
  )

set(CLP MultiResolutionAffineRegistration)
slicerMacroBuildCLI(
  NAME ${CLP}
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h
  TARGET_LIBRARIES ${ITK_LIBRARIES}
  INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
  ADDITIONAL_SRCS
    ScalingHeuristics.cxx
    ConvertSlicerROIToRegion.cxx
  )

if(BUILD_TESTING)
  add_subdirectory( Testing )
endif()
