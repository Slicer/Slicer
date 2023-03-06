set(proj python-numpy)

# Set dependency list
set(${proj}_DEPENDENCIES
  python
  python-pip
  python-setuptools
  )

if(NOT DEFINED Slicer_USE_SYSTEM_${proj})
  set(Slicer_USE_SYSTEM_${proj} ${Slicer_USE_SYSTEM_python})
endif()

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

if(Slicer_USE_SYSTEM_${proj})
  foreach(module_name IN ITEMS
    nose
    numpy
    )
    ExternalProject_FindPythonPackage(
      MODULE_NAME "${module_name}"
      REQUIRED
      )
  endforeach()
endif()

if(NOT Slicer_USE_SYSTEM_${proj})
  set(requirements_file ${CMAKE_BINARY_DIR}/${proj}-requirements.txt)
  file(WRITE ${requirements_file} [===[
  # [nose]
  nose==1.3.7 --hash=sha256:9ff7c6cc443f8c51994b34a667bbcf45afd6d945be7477b52e97516fd17c53ac  # needed for NumPy unit tests
  # [/nose]
  # [numpy]
  # Hashes correspond to the following packages:
  #  - numpy-1.24.2-cp39-cp39-macosx_10_9_x86_64.whl
  #  - numpy-1.24.2-cp39-cp39-macosx_11_0_arm64.whl
  #  - numpy-1.24.2-cp39-cp39-manylinux_2_17_aarch64.manylinux2014_aarch64.whl
  #  - numpy-1.24.2-cp39-cp39-manylinux_2_17_x86_64.manylinux2014_x86_64.whl
  #  - numpy-1.24.2-cp39-cp39-win_amd64.whl
  numpy==1.24.2 --hash=sha256:4199e7cfc307a778f72d293372736223e39ec9ac096ff0a2e64853b866a8e18a \
                --hash=sha256:adbdce121896fd3a17a77ab0b0b5eedf05a9834a18699db6829a64e1dfccca7f \
                --hash=sha256:889b2cc88b837d86eda1b17008ebeb679d82875022200c6e8e4ce6cf549b7acb \
                --hash=sha256:f64bb98ac59b3ea3bf74b02f13836eb2e24e48e0ab0145bbda646295769bd780 \
                --hash=sha256:a77d3e1163a7770164404607b7ba3967fb49b24782a6ef85d9b5f54126cc39e5
  # [/numpy]
  ]===])

  ExternalProject_Add(${proj}
    ${${proj}_EP_ARGS}
    DOWNLOAD_COMMAND ""
    SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ${PYTHON_EXECUTABLE} -m pip install --require-hashes -r ${requirements_file}
    LOG_INSTALL 1
    DEPENDS
      ${${proj}_DEPENDENCIES}
    )

  ExternalProject_GenerateProjectDescription_Step(${proj}
    VERSION ${_version}
    )

  #-----------------------------------------------------------------------------
  # Sanity checks

  foreach(varname IN ITEMS
      python_DIR
      PYTHON_SITE_PACKAGES_SUBDIR
      )
    if("${${varname}}" STREQUAL "")
      message(FATAL_ERROR "${varname} CMake variable is expected to be set")
    endif()
  endforeach()

  #-----------------------------------------------------------------------------
  # Launcher setting specific to build tree

  set(${proj}_LIBRARY_PATHS_LAUNCHER_BUILD
    ${python_DIR}/${PYTHON_SITE_PACKAGES_SUBDIR}/numpy/core
    ${python_DIR}/${PYTHON_SITE_PACKAGES_SUBDIR}/numpy/lib
    )
  mark_as_superbuild(
    VARS ${proj}_LIBRARY_PATHS_LAUNCHER_BUILD
    LABELS "LIBRARY_PATHS_LAUNCHER_BUILD"
    )

  #-----------------------------------------------------------------------------
  # Launcher setting specific to install tree

  set(${proj}_LIBRARY_PATHS_LAUNCHER_INSTALLED
    <APPLAUNCHER_SETTINGS_DIR>/../lib/Python/${PYTHON_SITE_PACKAGES_SUBDIR}/numpy/core
    <APPLAUNCHER_SETTINGS_DIR>/../lib/Python/${PYTHON_SITE_PACKAGES_SUBDIR}/numpy/lib
    )
  mark_as_superbuild(
    VARS ${proj}_LIBRARY_PATHS_LAUNCHER_INSTALLED
    LABELS "LIBRARY_PATHS_LAUNCHER_INSTALLED"
    )

else()
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
endif()

