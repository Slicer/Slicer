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
  #  - numpy-1.23.1-cp39-cp39-macosx_10_9_x86_64.whl
  #  - numpy-1.23.1-cp39-cp39-macosx_11_0_arm64.whl
  #  - numpy-1.23.1-cp39-cp39-manylinux_2_17_aarch64.manylinux2014_aarch64.whl
  #  - numpy-1.23.1-cp39-cp39-manylinux_2_17_x86_64.manylinux2014_x86_64.whl
  #  - numpy-1.23.1-cp39-cp39-win_amd64.whl
  numpy==1.23.1 --hash=sha256:173f28921b15d341afadf6c3898a34f20a0569e4ad5435297ba262ee8941e77b \
                --hash=sha256:876f60de09734fbcb4e27a97c9a286b51284df1326b1ac5f1bf0ad3678236b22 \
                --hash=sha256:35590b9c33c0f1c9732b3231bb6a72d1e4f77872390c47d50a615686ae7ed3fd \
                --hash=sha256:a35c4e64dfca659fe4d0f1421fc0f05b8ed1ca8c46fb73d9e5a7f175f85696bb \
                --hash=sha256:37ece2bd095e9781a7156852e43d18044fd0d742934833335599c583618181b9
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

