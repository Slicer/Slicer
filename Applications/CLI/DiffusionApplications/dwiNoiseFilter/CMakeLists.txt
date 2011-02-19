project( dwiNoiseFilter )

find_package(GenerateCLP REQUIRED)
include(${GenerateCLP_USE_FILE})

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

slicerMacroBuildCLI(
  NAME ${PROJECT_NAME}
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h
  TARGET_LIBRARIES ${ITK_LIBRARIES} ModuleDescriptionParser
  LINK_DIRECTORIES ${ModuleDescriptionParser_BINARY_DIR}
  INCLUDE_DIRECTORIES
    ${SlicerBaseCLI_SOURCE_DIR} ${SlicerBaseCLI_BINARY_DIR}
  )

