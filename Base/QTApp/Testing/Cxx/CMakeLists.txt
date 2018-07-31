set(KIT ${PROJECT_NAME})

set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
set(KIT_TEST_SRCS
  qSlicerAppAboutDialogTest1.cxx
  )
create_test_sourcelist(Tests ${KIT}CxxTests.cxx
  ${KIT_TEST_SRCS}
  EXTRA_INCLUDE vtkMRMLDebugLeaksMacro.h
  )

set(target_libraries ${KIT})

add_executable(${KIT}CxxTests ${Tests})
target_link_libraries(${KIT}CxxTests  ${target_libraries})
set_target_properties(${KIT}CxxTests PROPERTIES LABELS ${KIT})
set_target_properties(${KIT}CxxTests PROPERTIES FOLDER "Core-Base")

#
# Add Tests
#

simple_test( qSlicerAppAboutDialogTest1 )
