
if(NOT APPLE)
  message(FATAL_ERROR "This module was designed for macOS")
endif()

# Install VTK python module
if(Slicer_VTK_VERSION_MAJOR VERSION_GREATER "7")
  set(VTK_PYTHON_MODULE "${VTK_DIR}/lib/python2.7/site-packages")
else()
  set(VTK_PYTHON_MODULE "${VTK_DIR}/Wrapping/Python")
endif()
install(DIRECTORY ${VTK_PYTHON_MODULE}/vtk
  DESTINATION ${Slicer_INSTALL_BIN_DIR}/Python
  USE_SOURCE_PERMISSIONS
  COMPONENT Runtime
  # VTK9: Add exclusions to avoid installing VTK's C++ Python modules
  # These have their own install rules (see below).
  PATTERN "vtk*Python.so" EXCLUDE)

# Install external python runtime libraries that we don't link to (fixupbundle won't copy them)
if(Slicer_VTK_VERSION_MAJOR VERSION_GREATER "7")
  set(vtk_python_library_subdir "lib/python2.7/site-packages/vtk")
else()
  set(vtk_python_library_subdir "lib")
endif()
file(GLOB vtk_python_modules "${VTK_DIR}/${vtk_python_library_subdir}/*Python.so")
install(FILES ${vtk_python_modules}
        DESTINATION ${Slicer_INSTALL_LIB_DIR}
        COMPONENT Runtime)

# Install CTK python modules
install(DIRECTORY ${CTK_DIR}/CTK-build/bin/Python/ctk ${CTK_DIR}/CTK-build/bin/Python/qt
  DESTINATION ${Slicer_INSTALL_BIN_DIR}/Python
  USE_SOURCE_PERMISSIONS
  COMPONENT Runtime)

