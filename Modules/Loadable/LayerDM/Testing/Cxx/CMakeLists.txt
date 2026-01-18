set(KIT ${MODULE_NAME})
find_package(Qt5 REQUIRED COMPONENTS Core Widgets Test)

set(TEST_SOURCES
  NodeReferenceObserverTest.cxx
)

set(EXTRA_INCLUDE "vtkMRMLDebugLeaksMacro.h\"\n\#include <itkConfigure.h>\n\#include <itkFactoryRegistration.h>\n\#include \"vtkTestingOutputWindow.h")
create_test_sourcelist(Tests ${KIT}CppTests.cxx
  ${TEST_SOURCES}
  EXTRA_INCLUDE ${EXTRA_INCLUDE}
)

QT5_GENERATE_MOCS(${TEST_SOURCES} USE_MOC_EXTENSION)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
ctk_add_executable_utf8(${KIT}CxxTests ${Tests} ${Tests_MOC_CXX} ${Tests_UtilityFiles})

target_link_libraries(
  ${KIT}CxxTests
  ${MODULE_TARGET_LIBRARIES}
  Qt5::Core
  Qt5::Test
  Qt5::Widgets
)

include(SlicerMacroSimpleTest)
simple_test(NodeReferenceObserverTest)
