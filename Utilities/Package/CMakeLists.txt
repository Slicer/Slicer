
if(APPLE)


  get_filename_component(SUPER_BUILD_DIR "${Slicer_BINARY_DIR}" PATH)
  set(PYTHON_DIR "${SUPER_BUILD_DIR}/python-build")

  if(EXISTS "${PYTHON_DIR}")
    install(DIRECTORY
      "${PYTHON_DIR}/"
      DESTINATION ${Slicer_INSTALL_ROOT}/lib/Python
      USE_SOURCE_PERMISSIONS
      REGEX "bin/python" EXCLUDE
      REGEX "include/" EXCLUDE
      REGEX "share/man" EXCLUDE
      REGEX "command/wininst" EXCLUDE
      )
  endif()

  # install vtk python module as well
  list(GET VTK_PYTHONPATH_DIRS 0 VTK_PYTHON_MODULE)
  install(DIRECTORY ${VTK_PYTHON_MODULE}/vtk
    DESTINATION ${Slicer_INSTALL_BIN_DIR}/Python
    USE_SOURCE_PERMISSIONS
    COMPONENT Runtime)

  list(GET VTK_PYTHONPATH_DIRS 0 VTK_PYTHON_MODULE)

  install(DIRECTORY ${CTK_DIR}/CTK-build/bin/Python/ctk ${CTK_DIR}/CTK-build/bin/Python/qt
    DESTINATION ${Slicer_INSTALL_BIN_DIR}/Python
    USE_SOURCE_PERMISSIONS
    COMPONENT Runtime)

  # install external python runtime libraries that we don't link to (fixupbundle won't copy them)
  file(GLOB vtk_python_modules "${VTK_DIR}/bin/*Python.so")
  install(FILES ${vtk_python_modules}
          DESTINATION ${Slicer_INSTALL_LIB_DIR}
          COMPONENT Runtime)

  include(${Slicer_CMAKE_DIR}/SlicerBlockInstallQtImageFormatsPlugins.cmake)

  set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

  set(executable_path @executable_path)

  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/CompleteBundles.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/CompleteBundles.cmake"
    @ONLY)

  install(SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/CompleteBundles.cmake")

  set(CPACK_GENERATOR "DragNDrop")

endif(APPLE)
