SET(KIT Annotation)

SET(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
CREATE_TEST_SOURCELIST(Tests ${KIT}CxxTests.cxx
  #qMRMLSceneAnnotationModelAndAnnotationTreeWidgetTest1
  vtkMRMLAnnotationNodeTest1.cxx
  vtkMRMLAnnotationStorageNodeTest1.cxx
  vtkMRMLAnnotationDisplayNodeTest1.cxx
  vtkMRMLAnnotationControlPointsNodeTest1.cxx
  vtkMRMLAnnotationPointDisplayNodeTest1.cxx
  vtkMRMLAnnotationControlPointsStorageNodeTest1.cxx
  vtkMRMLAnnotationLinesNodeTest1.cxx
  vtkMRMLAnnotationLineDisplayNodeTest1.cxx
  vtkMRMLAnnotationLinesStorageNodeTest1.cxx
  vtkMRMLAnnotationAngleNodeTest1.cxx
  vtkMRMLAnnotationAngleStorageNodeTest1.cxx
  vtkMRMLAnnotationRulerNodeTest1.cxx
  vtkMRMLAnnotationRulerStorageNodeTest1.cxx
  vtkMRMLAnnotationFiducialNodeTest1.cxx
  vtkMRMLAnnotationFiducialsStorageNodeTest1.cxx
  #TestAnnotationWidgetsStandalone.cxx
  #vtkMRMLAnnotationTextNodeAndWidgetTest1.cxx
  #vtkMRMLAnnotationTextDisplayableManagerTest1.cxx
  #vtkMRMLAnnotationRulerDisplayableManagerTest1.cxx
  #vtkMRMLAnnotationTextNodeAndWidgetTest1.cxx
  #vtkSlicerAnnotationModuleLogicTest1.cxx
  #qSlicerAnnotationModuleAnnotationPropertyDialogTest1.cxx
  #TestAnnAngleWidget3D.cxx
  EXTRA_INCLUDE TestingMacros.h
  )

SET (TestsToRun ${Tests})
REMOVE (TestsToRun ${KIT}CxxTests.cxx)

SET(LIBRARY_NAME ${PROJECT_NAME})

ADD_EXECUTABLE(${KIT}CxxTests ${Tests})
TARGET_LINK_LIBRARIES(${KIT}CxxTests ${lib_name})

#MACRO(SIMPLE_TEST  TESTNAME)
#  ADD_TEST(NAME ${TESTNAME} COMMAND ${LAUNCH_EXE} $<TARGET_FILE:${KIT}CxxTests> ${TESTNAME})
#ENDMACRO()

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


#ADD_TEST(Slicer3AnnotationRulerTest1 ${Slicer_BINARY_DIR}/Slicer3 --test-mode --script ${Slicer_SOURCE_DIR}/QTModules/Annotation/Testing/TestAddRuler.tcl)

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
#     ${CMAKE_BINARY_DIR}/${Slicer_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME}/Testing/${filename}
#     COPYONLY)
# endforeach(file)
# 
# install(FILES 
#   ${MRMLFILES} 
#   ${NHDRFILES} 
#   ${RAWGZFILES}
#   ${LOGFILES} 
#   ${TESTFILES}
#   DESTINATION ${Slicer_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME}/Testing COMPONENT Development
#   ) 

set(TEMP "${Slicer_BINARY_DIR}/Testing/Temporary")

#
# Add all the tests
#
FOREACH (test ${TestsToRun})
  GET_FILENAME_COMPONENT(TESTNAME ${test} NAME_WE)
  ADD_TEST(
    NAME ${TESTNAME} 
    COMMAND ${LAUNCH_EXE} $<TARGET_FILE:${KIT}CxxTests> ${TESTNAME}
      -D ${CMAKE_CURRENT_SOURCE_DIR}/../
      -T ${TEMP}
      #-V Baseline/${TESTNAME}.png
    )
ENDFOREACH()
