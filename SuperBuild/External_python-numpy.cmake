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
  #  - numpy-1.22.2-cp39-cp39-macosx_10_14_x86_64.whl
  #  - numpy-1.22.2-cp39-cp39-macosx_11_0_arm64.whl
  #  - numpy-1.22.2-cp39-cp39-manylinux_2_17_aarch64.manylinux2014_aarch64.whl
  #  - numpy-1.22.2-cp39-cp39-manylinux_2_17_x86_64.manylinux2014_x86_64.whl
  #  - numpy-1.22.2-cp39-cp39-win_amd64.whl
  numpy==1.22.2 --hash=sha256:d76a26c5118c4d96e264acc9e3242d72e1a2b92e739807b3b69d8d47684b6677 \
                --hash=sha256:15efb7b93806d438e3bc590ca8ef2f953b0ce4f86f337ef4559d31ec6cf9d7dd \
                --hash=sha256:badca914580eb46385e7f7e4e426fea6de0a37b9e06bec252e481ae7ec287082 \
                --hash=sha256:94dd11d9f13ea1be17bac39c1942f527cbf7065f94953cf62dfe805653da2f8f \
                --hash=sha256:59153979d60f5bfe9e4c00e401e24dfe0469ef8da6d68247439d3278f30a180f
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

