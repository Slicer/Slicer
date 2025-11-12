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
  #  - numpy-2.3.4-cp312-cp312-macosx_10_13_x86_64.whl
  #  - numpy-2.3.4-cp312-cp312-macosx_11_0_arm64.whl
  #  - numpy-2.3.4-cp312-cp312-macosx_14_0_arm64.whl
  #  - numpy-2.3.4-cp312-cp312-macosx_14_0_x86_64.whl
  #  - numpy-2.3.4-cp312-cp312-manylinux_2_27_aarch64.manylinux_2_28_aarch64.whl
  #  - numpy-2.3.4-cp312-cp312-manylinux_2_27_x86_64.manylinux_2_28_x86_64.whl
  #  - numpy-2.3.4-cp312-cp312-musllinux_1_2_aarch64.whl
  #  - numpy-2.3.4-cp312-cp312-musllinux_1_2_x86_64.whl
  #  - numpy-2.3.4-cp312-cp312-win_amd64.whl
  #  - numpy-2.3.4-cp312-cp312-win_arm64.whl
  numpy==2.3.4 --hash=sha256:ef1b5a3e808bc40827b5fa2c8196151a4c5abe110e1726949d7abddfe5c7ae11 \
               --hash=sha256:c2f91f496a87235c6aaf6d3f3d89b17dba64996abadccb289f48456cff931ca9 \
               --hash=sha256:f77e5b3d3da652b474cc80a14084927a5e86a5eccf54ca8ca5cbd697bf7f2667 \
               --hash=sha256:8ab1c5f5ee40d6e01cbe96de5863e39b215a4d24e7d007cad56c7184fdf4aeef \
               --hash=sha256:77b84453f3adcb994ddbd0d1c5d11db2d6bda1a2b7fd5ac5bd4649d6f5dc682e \
               --hash=sha256:4121c5beb58a7f9e6dfdee612cb24f4df5cd4db6e8261d7f4d7450a997a65d6a \
               --hash=sha256:65611ecbb00ac9846efe04db15cbe6186f562f6bb7e5e05f077e53a599225d16 \
               --hash=sha256:dabc42f9c6577bcc13001b8810d300fe814b4cfbe8a92c873f269484594f9786 \
               --hash=sha256:985f1e46358f06c2a09921e8921e2c98168ed4ae12ccd6e5e87a4f1857923f32 \
               --hash=sha256:4635239814149e06e2cb9db3dd584b2fa64316c96f10656983b8026a82e6e4db
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
