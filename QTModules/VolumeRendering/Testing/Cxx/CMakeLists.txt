set(KIT VolumeRendering)

set(KIT_TEST_SRCS)
set(KIT_TEST_NAMES)
set(KIT_TEST_NAMES_CXX)
SlicerMacroConfigureGenericCxxModuleTests(${KIT} KIT_TEST_SRCS KIT_TEST_NAMES KIT_TEST_NAMES_CXX)

set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
create_test_sourcelist(Tests ${KIT}CxxTests.cxx
  ${KIT_TEST_NAMES_CXX}
  qSlicerVolumeRenderingModuleWidgetTest1.cxx
  qSlicerVolumeRenderingModuleWidgetTest2.cxx
  vtkMRMLVolumeRenderingDisplayableManagerTest1.cxx
  EXTRA_INCLUDE TestingMacros.h
  )
list(REMOVE_ITEM Tests ${KIT_TEST_NAMES_CXX})
list(APPEND Tests ${KIT_TEST_SRCS})

#set(TestsToRun ${Tests})
#list(REMOVE_ITEM TestsToRun ${KIT}CxxTests.cxx)

set(LIBRARY_NAME ${PROJECT_NAME})

add_executable(${KIT}CxxTests ${Tests})
target_link_libraries(${KIT}CxxTests ${lib_name} vtkSlicerVolumesModuleLogic)

set(VOLUME_FILE "${MRML_SOURCE_DIR}/Testing/TestData/fixed.nrrd")

macro(SIMPLE_TEST  TESTNAME)
  add_test(NAME ${TESTNAME} COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:${KIT}CxxTests> ${TESTNAME} ${ARGN})
endmacro()

foreach(testname ${KIT_TEST_NAMES})
  SIMPLE_TEST( ${testname} )
endforeach()
SIMPLE_TEST( qSlicerVolumeRenderingModuleWidgetTest1 )
SIMPLE_TEST( qSlicerVolumeRenderingModuleWidgetTest2 ${VOLUME_FILE} )
SIMPLE_TEST( vtkMRMLVolumeRenderingDisplayableManagerTest1 )

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

#
# Add all the tests
#
#set(TEMP "${Slicer_BINARY_DIR}/Testing/Temporary")
#foreach(test ${TestsToRun})
#  get_filename_component(TESTNAME ${test} NAME_WE)
#  add_test(
#    NAME ${TESTNAME}
#    COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:${KIT}CxxTests> ${TESTNAME}
#      -D ${CMAKE_CURRENT_SOURCE_DIR}/../
#      -T ${TEMP}
#      #-V Baseline/${TESTNAME}.png
#    )
#endforeach()
