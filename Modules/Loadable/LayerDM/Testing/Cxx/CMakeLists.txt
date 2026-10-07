set(KIT qSlicer${MODULE_NAME}Module)

# NodeReferenceObserverTest builds its node references from Markups nodes.
slicer_is_loadable_builtin_module_enabled("Markups" _build_markups_module)
if(NOT _build_markups_module)
  return()
endif()

#-----------------------------------------------------------------------------
set(KIT_TEST_SRCS
  NodeReferenceObserverTest.cxx
  )

#-----------------------------------------------------------------------------
include_directories(${CMAKE_CURRENT_BINARY_DIR})

#-----------------------------------------------------------------------------
slicerMacroConfigureModuleCxxTestDriver(
  NAME ${KIT}
  SOURCES ${KIT_TEST_SRCS}
  TARGET_LIBRARIES vtkSlicerMarkupsModuleMRML
  WITH_VTK_DEBUG_LEAKS_CHECK
  WITH_VTK_ERROR_OUTPUT_CHECK
  )

# Configure CMake Qt automatic code generation
set_target_properties(${KIT}CxxTests PROPERTIES
  AUTOMOC ON
  )

#-----------------------------------------------------------------------------
simple_test(NodeReferenceObserverTest)
