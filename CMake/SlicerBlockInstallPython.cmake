# -------------------------------------------------------------------------
# Find and install python
# -------------------------------------------------------------------------
if(Slicer_USE_PYTHONQT)

  # Sanity checks
  foreach(varname IN ITEMS Slicer_SUPERBUILD_DIR PYTHON_STDLIB_SUBDIR)
    if("${${varname}}" STREQUAL "")
      message(FATAL_ERROR "${varname} CMake variable is expected to be set")
    endif()
  endforeach()

  set(PYTHON_DIR "${Slicer_SUPERBUILD_DIR}/python-install")
  if(NOT EXISTS "${PYTHON_DIR}/${PYTHON_STDLIB_SUBDIR}")
    message(STATUS "Skipping generation of python install rules - Unexistant directory PYTHON_DIR:${PYTHON_DIR}/${PYTHON_STDLIB_SUBDIR}")
    return()
  endif()

  # Install libraries
  
  set(extra_exclude_pattern)
  if(UNIX)
    list(APPEND extra_exclude_pattern
      REGEX "distutils/command/wininst-.*" EXCLUDE
      )
  endif()

  install(
    DIRECTORY "${PYTHON_DIR}/${PYTHON_STDLIB_SUBDIR}/"
    DESTINATION "${Slicer_INSTALL_ROOT}lib/Python/${PYTHON_STDLIB_SUBDIR}/"
    COMPONENT Runtime
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
    install(FILES "${PYTHON_LIBRARY_PATH}/${PYTHON_LIB_BASE}.dll"
      DESTINATION bin
      COMPONENT Runtime)
  endif()

  # Install interpreter
  get_filename_component(python_bin_dir ${PYTHON_EXECUTABLE} PATH)
  install(
    PROGRAMS ${python_bin_dir}/python${CMAKE_EXECUTABLE_SUFFIX}
    DESTINATION ${Slicer_INSTALL_BIN_DIR}
    RENAME python-real${CMAKE_EXECUTABLE_SUFFIX}
    COMPONENT Runtime
    )

  # Install Slicer python launcher settings
  install(
    FILES ${python_bin_dir}/SlicerPythonLauncherSettingsToInstall.ini
    DESTINATION ${Slicer_INSTALL_BIN_DIR}
    RENAME SlicerPythonLauncherSettings.ini
    COMPONENT Runtime
    )

  # Install Slicer python launcher
  set(_launcher CTKAppLauncher)
  if(Slicer_BUILD_WIN32_CONSOLE)
    set(_launcher CTKAppLauncherW)
  endif()
  install(
    PROGRAMS ${CTKAPPLAUNCHER_DIR}/bin/${_launcher}${CMAKE_EXECUTABLE_SUFFIX}
    DESTINATION ${Slicer_INSTALL_BIN_DIR}
    RENAME SlicerPython${CMAKE_EXECUTABLE_SUFFIX}
    COMPONENT Runtime
    )

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

