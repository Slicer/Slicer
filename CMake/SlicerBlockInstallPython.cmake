# -------------------------------------------------------------------------
# Find and install python
# -------------------------------------------------------------------------
if(Slicer_USE_PYTHON OR Slicer_USE_PYTHONQT)

  get_filename_component(SUPER_BUILD_DIR "${Slicer_BINARY_DIR}" PATH)
  set(PYTHON_DIR "${SUPER_BUILD_DIR}/python-build")
  set(python_lib_subdir /Lib/)
  if(UNIX)
    set(python_lib_subdir /lib/python2.6/)
  endif()
  if(NOT EXISTS "${PYTHON_DIR}${python_lib_subdir}")
    message(FATAL_ERROR "error: Failed to install Python ! - Unexistant directory PYTHON_DIR:${PYTHON_DIR}${python_lib_subdir}")
  endif()

  install(
    DIRECTORY "${PYTHON_DIR}${python_lib_subdir}"
    DESTINATION ${Slicer_INSTALL_ROOT}lib/Python${python_lib_subdir}
    USE_SOURCE_PERMISSIONS
    REGEX "lib2to3/" EXCLUDE
    REGEX "lib-old/" EXCLUDE
    REGEX "plat-*" EXCLUDE
    REGEX "test/" EXCLUDE
    REGEX "wsgiref*" EXCLUDE
    REGEX "distutils/" EXCLUDE
    )
  # Install python library
  if(UNIX)
    slicerInstallLibrary(
      FILE ${PYTHON_LIBRARY}
      DESTINATION ${Slicer_INSTALL_ROOT}lib/Python/lib
      COMPONENT Runtime
      PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ)
  elseif(WIN32)
    get_filename_component(PYTHON_LIB_BASE ${PYTHON_LIBRARY} NAME_WE)
    get_filename_component(PYTHON_LIB_PATH ${PYTHON_LIBRARY} PATH)
    install(FILES "${PYTHON_LIB_PATH}/${PYTHON_LIB_BASE}.dll"
      DESTINATION bin
      COMPONENT Runtime)
  endif()
endif()

