project(ResampleDTI)

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

slicerMacroBuildCLI(
  NAME ${PROJECT_NAME}
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h
  ADDITIONAL_SRCS
    dtiprocessFiles/deformationfieldio.h
    dtiprocessFiles/deformationfieldio.cxx
    dtiprocessFiles/itkHFieldToDeformationFieldImageFilter.h
    dtiprocessFiles/itkHFieldToDeformationFieldImageFilter.txx
  TARGET_LIBRARIES ModuleDescriptionParser ${ITK_LIBRARIES}
  LINK_DIRECTORIES ${ModuleDescriptionParser_BINARY_DIR}
  INCLUDE_DIRECTORIES
    ${SlicerBaseCLI_SOURCE_DIR} ${SlicerBaseCLI_BINARY_DIR}
  )

if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
