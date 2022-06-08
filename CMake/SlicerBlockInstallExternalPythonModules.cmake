
if(NOT APPLE)
  message(FATAL_ERROR "This module was designed for macOS")
endif()

# Install VTK python module
set(VTK_PYTHON_MODULE "${VTK_DIR}/lib/python${Slicer_PYTHON_VERSION_DOT}/site-packages")
install(DIRECTORY ${VTK_PYTHON_MODULE}/vtkmodules
  DESTINATION ${Slicer_INSTALL_BIN_DIR}/Python
  USE_SOURCE_PERMISSIONS
  COMPONENT Runtime)
install(FILES ${VTK_PYTHON_MODULE}/vtk.py
  DESTINATION ${Slicer_INSTALL_BIN_DIR}/Python
  COMPONENT Runtime)

# Install CTK python modules
install(DIRECTORY ${CTK_DIR}/CTK-build/bin/Python/ctk ${CTK_DIR}/CTK-build/bin/Python/qt
  DESTINATION ${Slicer_INSTALL_BIN_DIR}/Python
  USE_SOURCE_PERMISSIONS
  COMPONENT Runtime)

