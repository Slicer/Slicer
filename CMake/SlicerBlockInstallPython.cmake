# -------------------------------------------------------------------------
# Find and install python
# -------------------------------------------------------------------------
if(Slicer_USE_PYTHONQT)

  set(python_lib_subdir /Lib/)
  if(UNIX)
    set(python_lib_subdir /lib/python2.7/)
  endif()

  get_filename_component(SUPER_BUILD_DIR "${Slicer_BINARY_DIR}" PATH)
  set(PYTHON_DIR "${SUPER_BUILD_DIR}/python-install")
  if(NOT EXISTS "${PYTHON_DIR}${python_lib_subdir}")
    message(FATAL_ERROR "error: Failed to install Python ! - Unexistant directory PYTHON_DIR:${PYTHON_DIR}${python_lib_subdir}")
  endif()

  # Install libraries
  
  set(extra_exclude_pattern)
  if(UNIX)
    list(APPEND extra_exclude_pattern
      REGEX "distutils/command/wininst-.*" EXCLUDE
      )
  endif()

  install(
    DIRECTORY "${PYTHON_DIR}${python_lib_subdir}"
    DESTINATION ${Slicer_INSTALL_ROOT}lib/Python${python_lib_subdir}
    USE_SOURCE_PERMISSIONS
    REGEX "lib2to3/" EXCLUDE
    REGEX "lib[-]old/" EXCLUDE
    REGEX "plat[-].*" EXCLUDE
    REGEX "/test/" EXCLUDE
    REGEX "wsgiref*" EXCLUDE
    ${extra_exclude_pattern}
    )
  # Install python library
  if(UNIX)
    if(NOT APPLE)
      slicerInstallLibrary(
        FILE ${PYTHON_LIBRARY}
        DESTINATION ${Slicer_INSTALL_ROOT}lib/Python/lib
        COMPONENT Runtime
        PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ)
    endif()
  elseif(WIN32)
    get_filename_component(PYTHON_LIB_BASE ${PYTHON_LIBRARY} NAME_WE)
    get_filename_component(PYTHON_LIB_PATH ${PYTHON_EXECUTABLE} PATH)
    install(FILES "${PYTHON_LIB_PATH}/${PYTHON_LIB_BASE}.dll"
      DESTINATION bin
      COMPONENT Runtime)
  endif()

  # Install headers
  set(python_include_subdir /Include/)
  if(UNIX)
    set(python_include_subdir /include/python2.7/)
  endif()

  install(FILES "${PYTHON_DIR}${python_include_subdir}/pyconfig.h"
    DESTINATION ${Slicer_INSTALL_ROOT}lib/Python${python_include_subdir}
    COMPONENT Runtime
    )

endif()

