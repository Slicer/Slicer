set(KIT qSlicer${qt_module_name}Module)

set(KIT_TEST_SRCS)
set(KIT_TEST_NAMES)
set(KIT_TEST_NAMES_CXX)
SlicerMacroConfigureGenericCxxModuleTests(${qt_module_name} KIT_TEST_SRCS KIT_TEST_NAMES KIT_TEST_NAMES_CXX)

set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
create_test_sourcelist(Tests ${KIT}CxxTests.cxx
  ${KIT_TEST_NAMES_CXX}
  #qMRMLSceneAnnotationModelAndAnnotationTreeWidgetTest1
  qMRMLAnnotationROIWidgetTest1.cxx
  vtkMRMLAnnotationNodeTest1.cxx
  vtkMRMLAnnotationNodesUndoTest1.cxx
  vtkMRMLAnnotationStorageNodeTest1.cxx
  vtkMRMLAnnotationDisplayNodeTest1.cxx
  vtkMRMLAnnotationDisplayNodeTest2.cxx
  vtkMRMLAnnotationControlPointsNodeTest1.cxx
  vtkMRMLAnnotationPointDisplayNodeTest1.cxx
  vtkMRMLAnnotationControlPointsStorageNodeTest1.cxx
  vtkMRMLAnnotationTextDisplayNodeTest1.cxx
  vtkMRMLAnnotationLinesNodeTest1.cxx
  vtkMRMLAnnotationLineDisplayNodeTest1.cxx
  vtkMRMLAnnotationLinesStorageNodeTest1.cxx
  vtkMRMLAnnotationAngleNodeTest1.cxx
  vtkMRMLAnnotationAngleStorageNodeTest1.cxx
  vtkMRMLAnnotationRulerNodeTest1.cxx
  vtkMRMLAnnotationRulerStorageNodeTest1.cxx
  vtkMRMLAnnotationFiducialNodeTest1.cxx
  vtkMRMLAnnotationFiducialsStorageNodeTest1.cxx
  vtkMRMLAnnotationROINodeTest1.cxx
  vtkMRMLAnnotationDisplayableManagerTest1.cxx
  vtkMRMLAnnotationDisplayableManagerTest2.cxx
  #TestAnnotationWidgetsStandalone.cxx
  #vtkMRMLAnnotationTextNodeAndWidgetTest1.cxx
  #vtkMRMLAnnotationTextDisplayableManagerTest1.cxx
  #vtkMRMLAnnotationRulerDisplayableManagerTest1.cxx
  #vtkMRMLAnnotationTextNodeAndWidgetTest1.cxx
  vtkSlicerAnnotationModuleLogicTest1.cxx
  #qSlicerAnnotationModuleAnnotationPropertyDialogTest1.cxx
  #TestAnnAngleWidget3D.cxx
  vtkMRMLAnnotationHierarchyStorageNodeTest1.cxx
  EXTRA_INCLUDE TestingMacros.h
  )

list(REMOVE_ITEM Tests ${KIT_TEST_NAMES_CXX})
list(APPEND Tests ${KIT_TEST_SRCS})

set(TestsToRun ${Tests})
list(REMOVE_ITEM TestsToRun ${KIT}CxxTests.cxx)

add_executable(${KIT}CxxTests ${Tests})

set(target_libraries
  qSlicerAnnotationsModuleWidgets
  )

target_link_libraries(${KIT}CxxTests ${KIT}
  ${target_libraries}
  )

#SIMPLE_TEST( vtkMRMLAnnotationNodeTest1 )
#SIMPLE_TEST( vtkMRMLAnnotationStorageNodeTest1 )
#SIMPLE_TEST( vtkMRMLAnnotationDisplayNodeTest1 )
#SIMPLE_TEST( TestAnnAngleWidget3D )
#SIMPLE_TEST( vtkMRMLAnnotationControlPointsNodeTest1 )
#SIMPLE_TEST( vtkMRMLAnnotationPointDisplayNodeTest1 )
#SIMPLE_TEST( vtkMRMLAnnotationControlPointsStorageNodeTest1 )
#SIMPLE_TEST( vtkMRMLAnnotationLinesNodeTest1 )
#SIMPLE_TEST( vtkMRMLAnnotationLineDisplayNodeTest1 )
#SIMPLE_TEST( vtkMRMLAnnotationLinesStorageNodeTest1 )
#SIMPLE_TEST( vtkMRMLAnnotationAngleNodeTest1 )
#SIMPLE_TEST( vtkMRMLAnnotationAngleStorageNodeTest1 )
#SIMPLE_TEST( vtkMRMLAnnotationRulerNodeTest1 )
#SIMPLE_TEST( vtkMRMLAnnotationRulerStorageNodeTest1 )
#SIMPLE_TEST( vtkMRMLAnnotationFiducialNodeTest1 )
#SIMPLE_TEST( vtkMRMLAnnotationTextDisplayableManagerTest1 )
#SIMPLE_TEST( vtkSlicerAnnotationModuleLogicTest1 )
#SIMPLE_TEST( qSlicerAnnotationModuleAnnotationPropertyDialogTest1 )


#add_test(SlicerAnnotationRulerTest1 ${Slicer_LAUNCHER_EXECUTABLE} --test-mode --script ${Slicer_SOURCE_DIR}/Modules/Loadable/Annotation/Testing/TestAddRuler.tcl)

# SIMPLE_TEST( vtkMRMLAnnotationFiducialStorageNodeTest1 )
# Install the testing data


file(GLOB MRMLFILES "${CMAKE_CURRENT_SOURCE_DIR}/Testing/*.mrml")
file(GLOB NHDRFILES "${CMAKE_CURRENT_SOURCE_DIR}/Testing/*.nhdr")
file(GLOB RAWGZFILES "${CMAKE_CURRENT_SOURCE_DIR}/Testing/*.raw.gz")
file(GLOB LOGFILES "${CMAKE_CURRENT_SOURCE_DIR}/Testing/*.log")
file(GLOB TESTFILES "${CMAKE_CURRENT_SOURCE_DIR}/Testing/test*Script")

# foreach(file
#     ${MRMLFILES}
#     ${NHDRFILES}
#     ${RAWGZFILES}
#     ${LOGFILES}
#     ${TESTFILES}
#     )
# In your code, you just need to include: vtkSlicerConfigure.h
#  Then, the following 'define' will be available:
# Slicer_INSTALL_QTLOADABLEMODULES_BIN_DIR
# Slicer_INSTALL_QTLOADABLEMODULES_LIB_DIR
# Slicer_INSTALL_QTLOADABLEMODULES_INCLUDE_DIR
# Slicer_INSTALL_QTLOADABLEMODULES_SHARE_DIR
#
#   get_filename_component(filename "${file}" NAME)
#   configure_file(
#     ${file}
#     ${CMAKE_BINARY_DIR}/${Slicer_INSTALL_QTLOADABLEMODULES_SHARE_DIR}/${PROJECT_NAME}/Testing/${filename}
#     COPYONLY)
# endforeach(file)
#
# install(FILES
#   ${MRMLFILES}
#   ${NHDRFILES}
#   ${RAWGZFILES}
#   ${LOGFILES}
#   ${TESTFILES}
#   DESTINATION ${Slicer_INSTALL_QTLOADABLEMODULES_SHARE_DIR}/${PROJECT_NAME}/Testing COMPONENT Development
#   )

set(TEMP "${Slicer_BINARY_DIR}/Testing/Temporary")
MESSAGE(STATUS "TEMP = ${TEMP}, CMAKE_CURRENT_SOURCE_DIR = ${CMAKE_CURRENT_SOURCE_DIR}")

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Input/Fiducials\ List.acsv
  ${TEMP}/Fiducials\ List.acsv
  COPYONLY)

#
# Add all the tests
#
foreach(test ${TestsToRun})
  get_filename_component(TESTNAME ${test} NAME_WE)
  add_test(
    NAME ${TESTNAME}
    COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:${KIT}CxxTests> ${TESTNAME}
      -D ${CMAKE_CURRENT_SOURCE_DIR}/../
      -T ${TEMP}
      #-V Baseline/${TESTNAME}.png
    )
endforeach()
