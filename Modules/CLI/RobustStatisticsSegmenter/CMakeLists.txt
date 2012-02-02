project(RobustStatisticsSegmenter)

find_package(ITK REQUIRED)
include(${USE_ITK_FILE})

set(CLP RobustStatisticsSegmenter)

slicerMacroBuildCLI(
  NAME ${CLP}
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h
  TARGET_LIBRARIES ${ITK_LIBRARIES}
  ADDITIONAL_SRCS
    labelMapPreprocessor.h
    SFLS.h
    SFLSSegmentor3D.h
    SFLSSegmentor3D.txx
    SFLSRobustStatSegmentor3DLabelMap_single.h
    SFLSRobustStatSegmentor3DLabelMap_single.txx
  INCLUDE_DIRECTORIES
    ${vtkTeem_INCLUDE_DIRS}
    ${SlicerBaseCLI_SOURCE_DIR} ${SlicerBaseCLI_BINARY_DIR}
  )

if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
