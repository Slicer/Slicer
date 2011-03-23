SET(KIT Volumes)

SET(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
CREATE_TEST_SOURCELIST(Tests ${KIT}CxxTests.cxx
  qSlicerDiffusionTensorVolumeDisplayWidgetTest1.cxx
  qSlicerDTISliceDisplayWidgetTest1.cxx
  qSlicerDTISliceDisplayWidgetTest2.cxx
  EXTRA_INCLUDE TestingMacros.h
  )

SET(LIBRARY_NAME ${PROJECT_NAME})

ADD_EXECUTABLE(${KIT}CxxTests ${Tests})
TARGET_LINK_LIBRARIES(${KIT}CxxTests ${lib_name})

SET(DTI_FILE "${MRML_SOURCE_DIR}/Testing/TestData/helix-DTI.nhdr")

MACRO(SIMPLE_TEST  TESTNAME)
  ADD_TEST(NAME ${TESTNAME} COMMAND ${LAUNCH_EXE} $<TARGET_FILE:${KIT}CxxTests> ${TESTNAME})
ENDMACRO()

MACRO(TEST_WITH_ARG  TESTNAME ARG)
  ADD_TEST(NAME ${TESTNAME} COMMAND ${LAUNCH_EXE} $<TARGET_FILE:${KIT}CxxTests> ${TESTNAME} ${ARG})
ENDMACRO()

SIMPLE_TEST( qSlicerDiffusionTensorVolumeDisplayWidgetTest1 )
SIMPLE_TEST( qSlicerDTISliceDisplayWidgetTest1 )
TEST_WITH_ARG( qSlicerDTISliceDisplayWidgetTest2 ${DTI_FILE} )

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

#SET (TestsToRun ${Tests})
#REMOVE (TestsToRun ${KIT}CxxTests.cxx)

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
