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
  #  - numpy-2.4.5-cp312-cp312-macosx_10_13_x86_64.whl
  #  - numpy-2.4.5-cp312-cp312-macosx_11_0_arm64.whl
  #  - numpy-2.4.5-cp312-cp312-macosx_14_0_arm64.whl
  #  - numpy-2.4.5-cp312-cp312-macosx_14_0_x86_64.whl
  #  - numpy-2.4.5-cp312-cp312-manylinux_2_27_aarch64.manylinux_2_28_aarch64.whl
  #  - numpy-2.4.5-cp312-cp312-manylinux_2_27_x86_64.manylinux_2_28_x86_64.whl
  #  - numpy-2.4.5-cp312-cp312-musllinux_1_2_aarch64.whl
  #  - numpy-2.4.5-cp312-cp312-musllinux_1_2_x86_64.whl
  #  - numpy-2.4.5-cp312-cp312-win_amd64.whl
  #  - numpy-2.4.5-cp312-cp312-win_arm64.whl
  numpy==2.4.5 --hash=sha256:654fb8674b61b1c4bd568f944d13a908566fdcb0d797303521d4149d16da05ef \
               --hash=sha256:4cd9f6fa7ce10dc4627f2bb81dd9075dab67e94632e04c2b638e12575ddaa862 \
               --hash=sha256:4f5bc96d35d94e4ceab8b38a92241b4611e95dc44e63b9f1fa2a331858ee3507 \
               --hash=sha256:4bb33e900ee81730ad77a258965134aa8ceac805124f7e5229347beda4b8d0aa \
               --hash=sha256:32f8f852273ef32b291201ac2a2c97629c4a1ee8632bb670e3443eaa09fc2e72 \
               --hash=sha256:685681e956fc8dcb75adc6ff26694e1dfd738b24bd8d4696c51ca0110157f912 \
               --hash=sha256:6f64dd84b277a737eb59513f6b9bb6195bf41ab11941ef15b2562dbab43fa8ef \
               --hash=sha256:b42d9496f79e3a728192f05a42d86e36163217b7cdecb3813d0028a0aa6b72d7 \
               --hash=sha256:3333dba6a4e611d666f69e177ba8fe4140366ff681a5feb2374d3fd4fff3acb6 \
               --hash=sha256:4593d197270b894efeb538dcbe227e4bcf1c77f88c4c6bf933ead812cfaa4453
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
