project(RobustStatisticsSegmenter)

find_package(ITK REQUIRED)
include(${USE_ITK_FILE})

set(CLP RobustStatSegmenter)

slicerMacroBuildCLI(
  NAME ${CLP}
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h
  TARGET_LIBRARIES ${ITK_LIBRARIES}
  ADDITIONAL_SRCS
    #SFLSRobustStat3DTest.cxx
    RobustStatSegmenter.cxx
    SFLSRobustStat3DTestConsole.cxx
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

  add_executable( SFLSRobustStat3DTestConsole SFLSRobustStat3DTestConsole.cxx)
  target_link_libraries( SFLSRobustStat3DTestConsole ${ITK_LIBRARIES})
  set_target_properties(SFLSRobustStat3DTestConsole PROPERTIES LABELS ${CLP})

  set(testname ${CLP}Test)
  add_test(NAME ${testname} COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:SFLSRobustStat3DTestConsole>
      ${CMAKE_CURRENT_SOURCE_DIR}/grayscale.nrrd
      ${CMAKE_CURRENT_SOURCE_DIR}/grayscale-label.nrrd
      ${CMAKE_CURRENT_BINARY_DIR}/rss-test-seg.nrrd 50 0.1 0.2)
  set_property(TEST ${testname} PROPERTY LABELS ${CLP})
endif()
