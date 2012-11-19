find_package(DCMTK REQUIRED)

#-----------------------------------------------------------------------------
set(MODULE_NAME PETStandardUptakeValueComputation) # Do not use 'project()'

#-----------------------------------------------------------------------------
SEMMacroBuildCLI(
  NAME ${MODULE_NAME}
  ADDITIONAL_SRCS itkDCMTKFileReader.cxx
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h
  # LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/CTSCLogo.h
  TARGET_LIBRARIES vtkITK ModuleDescriptionParser
  MRMLCore SlicerBaseLogic SlicerBaseCLI ${ITK_LIBRARIES}
  ${DCMTK_LIBRARIES}
  INCLUDE_DIRECTORIES
    ${vtkITK_INCLUDE_DIRS}
    ${MRMLCore_INCLUDE_DIRS}
    ${SlicerBaseLogic_SOURCE_DIR}  ${SlicerBaseLogic_BINARY_DIR}
  EXECUTABLE_ONLY
  )

#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
