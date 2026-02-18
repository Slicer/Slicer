set(KIT ${MODULE_NAME})

list(APPEND QT_REQUIRED_COMPONENTS Core Test)
if(Slicer_REQUIRED_QT_VERSION VERSION_GREATER_EQUAL "6")
  find_package(Qt6 COMPONENTS ${QT_REQUIRED_COMPONENTS} QUIET)
else()
  find_package(Qt5 COMPONENTS ${QT_REQUIRED_COMPONENTS} QUIET)
endif()

set(TEST_SOURCES
  NodeReferenceObserverTest.cxx
)

set(EXTRA_INCLUDE "vtkMRMLDebugLeaksMacro.h\"\n\#include <itkConfigure.h>\n\#include <itkFactoryRegistration.h>\n\#include \"vtkTestingOutputWindow.h")
create_test_sourcelist(Tests ${KIT}CppTests.cxx
  ${TEST_SOURCES}
  EXTRA_INCLUDE ${EXTRA_INCLUDE}
)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
ctk_add_executable_utf8(${KIT}CxxTests ${Tests} ${Tests_MOC_CXX} ${Tests_UtilityFiles})

set_target_properties(${KIT}CxxTests PROPERTIES
  AUTOMOC ON
)

foreach(lib IN LISTS QT_REQUIRED_COMPONENTS)
  if(Slicer_REQUIRED_QT_VERSION VERSION_GREATER_EQUAL "6")
    list(APPEND MODULE_TARGET_LIBRARIES Qt6::${lib})
  else()
    list(APPEND MODULE_TARGET_LIBRARIES Qt5::${lib})
  endif()
endforeach()

target_link_libraries(
  ${KIT}CxxTests
  ${MODULE_TARGET_LIBRARIES}
  vtkSlicerMarkupsModuleMRML
)

include(SlicerMacroSimpleTest)
simple_test(NodeReferenceObserverTest)
