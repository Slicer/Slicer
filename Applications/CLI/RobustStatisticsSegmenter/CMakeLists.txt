project(RobustStatisticsSegmenter)

find_package(GenerateCLP REQUIRED)
include(${GenerateCLP_USE_FILE})

find_package(ITK REQUIRED)
include(${USE_ITK_FILE})

set(CLP SFLSRobustStat3DTest)

slicerMacroBuildCLI(
  NAME ${CLP}
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h
  TARGET_LIBRARIES ITKCommon ITKBasicFilters ITKIO itkvnl
  ADDITIONAL_SRCS
    SFLSRobustStat3DTest.cxx
    SFLSRobustStat3DTestConsole.cxx 
    labelMapPreprocessor.h 
    SFLS.h
    SFLSSegmentor3D.h 
    SFLSSegmentor3D.txx 
    SFLSRobustStatSegmentor3DLabelMap_single.h 
    SFLSRobustStatSegmentor3DLabelMap_single.txx
  INCLUDE_DIRECTORIES 
    ${MRML_SOURCE_DIR}          ${MRML_BINARY_DIR}
    ${vtkTeem_SOURCE_DIR}       ${vtkTeem_BINARY_DIR}
    ${SlicerBaseCLI_SOURCE_DIR} ${SlicerBaseCLI_BINARY_DIR}
  )

if(BUILD_TESTING)

  add_executable( SFLSRobustStat3DTestConsole SFLSRobustStat3DTestConsole.cxx)
  target_link_libraries( SFLSRobustStat3DTestConsole ITKCommon ITKBasicFilters ITKIO itkvnl)
  SET_TARGET_PROPERTIES(SFLSRobustStat3DTestConsole PROPERTIES LABELS ${CLP})
  
  set(Slicer_EXE ${Slicer_BINARY_DIR}/Slicer3 )
  # message (STATUS "cmake current source dir = '${CMAKE_CURRENT_SOURCE_DIR}'")
  # message (STATUS "cmake current bin dir = '${CMAKE_CURRENT_BINARY_DIR}'")
  # message (STATUS "slicer3 exe = '${Slicer_EXE}'")
  set(testname ${CLP}Test)
  add_test(${testname} ${Slicer_EXE}
    --launch SFLSRobustStat3DTestConsole 
      ${CMAKE_CURRENT_SOURCE_DIR}/grayscale.nrrd 
      ${CMAKE_CURRENT_SOURCE_DIR}/grayscale-label.nrrd 
      ${CMAKE_CURRENT_BINARY_DIR}/rss-test-seg.nrrd 50 0.1 0.2)
  SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})
ENDIF()
