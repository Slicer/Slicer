
# Install VTK python module
list(GET VTK_PYTHONPATH_DIRS 0 VTK_PYTHON_MODULE)
install(DIRECTORY ${VTK_PYTHON_MODULE}/vtk
  DESTINATION ${Slicer_INSTALL_BIN_DIR}/Python
  USE_SOURCE_PERMISSIONS
  COMPONENT Runtime)

# Install external python runtime libraries that we don't link to (fixupbundle won't copy them)
file(GLOB vtk_python_modules "${VTK_DIR}/bin/*Python.so")
install(FILES ${vtk_python_modules}
        DESTINATION ${Slicer_INSTALL_LIB_DIR}
        COMPONENT Runtime)

# Install CTK python modules
install(DIRECTORY ${CTK_DIR}/CTK-build/bin/Python/ctk ${CTK_DIR}/CTK-build/bin/Python/qt
  DESTINATION ${Slicer_INSTALL_BIN_DIR}/Python
  USE_SOURCE_PERMISSIONS
  COMPONENT Runtime)

