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
  # [numpy]
  # Hashes correspond to the following packages:
  #  - numpy-2.0.2-cp312-cp312-macosx_10_9_x86_64.whl
  #  - numpy-2.0.2-cp312-cp312-macosx_11_0_arm64.whl
  #  - numpy-2.0.2-cp312-cp312-macosx_14_0_arm64.whl
  #  - numpy-2.0.2-cp312-cp312-macosx_14_0_x86_64.whl
  #  - numpy-2.0.2-cp312-cp312-manylinux_2_17_aarch64.manylinux2014_aarch64.whl
  #  - numpy-2.0.2-cp312-cp312-manylinux_2_17_x86_64.manylinux2014_x86_64.whl
  #  - numpy-2.0.2-cp312-cp312-musllinux_1_1_x86_64.whl
  #  - numpy-2.0.2-cp312-cp312-musllinux_1_2_aarch64.whl
  #  - numpy-2.0.2-cp312-cp312-win_amd64.whl
  numpy==2.0.2 --hash=sha256:df55d490dea7934f330006d0f81e8551ba6010a5bf035a249ef61a94f21c500b \
               --hash=sha256:8df823f570d9adf0978347d1f926b2a867d5608f434a7cff7f7908c6570dcf5e \
               --hash=sha256:9a92ae5c14811e390f3767053ff54eaee3bf84576d99a2456391401323f4ec2c \
               --hash=sha256:a842d573724391493a97a62ebbb8e731f8a5dcc5d285dfc99141ca15a3302d0c \
               --hash=sha256:c05e238064fc0610c840d1cf6a13bf63d7e391717d247f1bf0318172e759e692 \
               --hash=sha256:0123ffdaa88fa4ab64835dcbde75dcdf89c453c922f18dced6e27c90d1d0ec5a \
               --hash=sha256:96a55f64139912d61de9137f11bf39a55ec8faec288c75a54f93dfd39f7eb40c \
               --hash=sha256:ec9852fb39354b5a45a80bdab5ac02dd02b15f44b3804e9f00c556bf24b4bded \
               --hash=sha256:cfd41e13fdc257aa5778496b8caa5e856dc4896d4ccf01841daee1d96465467a
  # [/numpy]
  ]===])

  ExternalProject_Add(${proj}
    ${${proj}_EP_ARGS}
    DOWNLOAD_COMMAND ""
    SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ${Python3_EXECUTABLE} -m pip install --require-hashes -r ${requirements_file}
    LOG_INSTALL 1
    DEPENDS
      ${${proj}_DEPENDENCIES}
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
