include(SlicerMacroBuildScriptedModule)
set(SUBJECTHIERARCHY_PYTHON_RESOURCES
  )

# Test subject hierarchy
slicerMacroBuildScriptedModule(
  NAME SubjectHierarchyGenericSelfTest
  SCRIPTS SubjectHierarchyGenericSelfTest.py
          SubjectHierarchyCorePluginsSelfTest.py
  RESOURCES ${SUBJECTHIERARCHY_PYTHON_RESOURCES}
  )

slicer_add_python_unittest(SCRIPT SubjectHierarchyGenericSelfTest.py
  SLICER_ARGS --disable-cli-modules
              #--no-main-window
              --additional-module-paths
                ${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_LIB_DIR}
                ${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_LIB_DIR}/Debug
                ${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_LIB_DIR}/Release
                ${CMAKE_BINARY_DIR}/${Slicer_QTSCRIPTEDMODULES_LIB_DIR}
  )

slicer_add_python_unittest(SCRIPT SubjectHierarchyCorePluginsSelfTest.py
  SLICER_ARGS --disable-cli-modules
              #--no-main-window
              --additional-module-paths
                ${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_LIB_DIR}
                ${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_LIB_DIR}/Debug
                ${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_LIB_DIR}/Release
                ${CMAKE_BINARY_DIR}/${Slicer_QTSCRIPTEDMODULES_LIB_DIR}
  )
