
if(NOT APPLE)
  message(FATAL_ERROR "This module was designed for macOS")
endif()

# Install VTK python module
set(VTK_PYTHON_MODULE "${VTK_DIR}/lib/python3.6/site-packages")
set(_vtk_package "vtk")
if(EXISTS ${VTK_PYTHON_MODULE}/vtkmodules)
  set(_vtk_package "vtkmodules") # Introduced in VTK9 kitware/vtk@2404228 on 2017.12.15
endif()
install(DIRECTORY ${VTK_PYTHON_MODULE}/${_vtk_package}
  DESTINATION ${Slicer_INSTALL_BIN_DIR}/Python
  USE_SOURCE_PERMISSIONS
  COMPONENT Runtime
  # VTK9: Add exclusions to avoid installing VTK's C++ Python modules
  # These have their own install rules (see below).
  PATTERN "vtk*Python.so" EXCLUDE
  PATTERN "*.pyo" EXCLUDE)

if(EXISTS ${VTK_PYTHON_MODULE}/vtk.py)
  # Introduced in VTK9 kitware/vtk@2404228 on 2017.12.15
  install(FILES ${VTK_PYTHON_MODULE}/vtk.py
    DESTINATION ${Slicer_INSTALL_BIN_DIR}/Python
    COMPONENT Runtime)
endif()

# Install external python runtime libraries that we don't link to (fixupbundle won't copy them)
set(vtk_python_library_subdir "lib/python3.6/site-packages/${_vtk_package}")
file(GLOB vtk_python_modules "${VTK_DIR}/${vtk_python_library_subdir}/*Python.so")
install(FILES ${vtk_python_modules}
        DESTINATION ${Slicer_INSTALL_LIB_DIR}
        COMPONENT Runtime)

# Install CTK python modules
install(DIRECTORY ${CTK_DIR}/CTK-build/bin/Python/ctk ${CTK_DIR}/CTK-build/bin/Python/qt
  DESTINATION ${Slicer_INSTALL_BIN_DIR}/Python
  USE_SOURCE_PERMISSIONS
  COMPONENT Runtime)

