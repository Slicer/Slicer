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
  #  - numpy-2.0.2-cp39-cp39-macosx_10_9_x86_64.whl
  #  - numpy-2.0.2-cp39-cp39-macosx_11_0_arm64.whl
  #  - numpy-2.0.2-cp39-cp39-macosx_14_0_arm64.whl
  #  - numpy-2.0.2-cp39-cp39-macosx_14_0_x86_64.whl
  #  - numpy-2.0.2-cp39-cp39-manylinux_2_17_aarch64.manylinux2014_aarch64.whl
  #  - numpy-2.0.2-cp39-cp39-manylinux_2_17_x86_64.manylinux2014_x86_64.whl
  #  - numpy-2.0.2-cp39-cp39-musllinux_1_1_x86_64.whl
  #  - numpy-2.0.2-cp39-cp39-musllinux_1_2_aarch64.whl
  #  - numpy-2.0.2-cp39-cp39-win_amd64.whl
  numpy==2.0.2 --hash=sha256:9059e10581ce4093f735ed23f3b9d283b9d517ff46009ddd485f1747eb22653c \
               --hash=sha256:423e89b23490805d2a5a96fe40ec507407b8ee786d66f7328be214f9679df6dd \
               --hash=sha256:2b2955fa6f11907cf7a70dab0d0755159bca87755e831e47932367fc8f2f2d0b \
               --hash=sha256:97032a27bd9d8988b9a97a8c4d2c9f2c15a81f61e2f21404d7e8ef00cb5be729 \
               --hash=sha256:1e795a8be3ddbac43274f18588329c72939870a16cae810c2b73461c40718ab1 \
               --hash=sha256:f26b258c385842546006213344c50655ff1555a9338e2e5e02a0756dc3e803dd \
               --hash=sha256:5fec9451a7789926bcf7c2b8d187292c9f93ea30284802a0ab3f5be8ab36865d \
               --hash=sha256:9189427407d88ff25ecf8f12469d4d39d35bee1db5d39fc5c168c6f088a6956d \
               --hash=sha256:a3f4ab0caa7f053f6797fcd4e1e25caee367db3112ef2b6ef82d749530768c73
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
