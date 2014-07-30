
# Install VTK python module
if(${VTK_VERSION_MAJOR} GREATER 5)
  set(VTK_PYTHON_MODULE "${VTK_DIR}/Wrapping/Python")
else()
  list(GET VTK_PYTHONPATH_DIRS 0 VTK_PYTHON_MODULE)
endif()
install(DIRECTORY ${VTK_PYTHON_MODULE}/vtk
  DESTINATION ${Slicer_INSTALL_BIN_DIR}/Python
  USE_SOURCE_PERMISSIONS
  COMPONENT Runtime)

# Install external python runtime libraries that we don't link to (fixupbundle won't copy them)
if(${VTK_VERSION_MAJOR} GREATER 5)
  if(UNIX)
    set(vtk_python_library_subdir "lib")
  else()
    set(vtk_python_library_subdir "bin")
  endif()
else()
  set(vtk_python_library_subdir "bin")
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

