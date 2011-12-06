
slicerMacroBuildCLI(
  NAME ExecutionModelTour
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/BillsLogo.h
  TARGET_LIBRARIES MRML
  INCLUDE_DIRECTORIES
    ${MRML_INCLUDE_DIRS}
  )

if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()

