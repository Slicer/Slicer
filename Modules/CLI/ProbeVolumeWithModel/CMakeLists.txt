
slicerMacroBuildCLI(
  NAME ProbeVolumeWithModel
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h
  TARGET_LIBRARIES MRMLCore vtkTeem SlicerBaseLogic
  INCLUDE_DIRECTORIES
    ${vtkITK_INCLUDE_DIRS}
    ${MRMLCore_INCLUDE_DIRS}
    ${vtkTeem_INCLUDE_DIRS}
  EXECUTABLE_ONLY
  )

if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()

