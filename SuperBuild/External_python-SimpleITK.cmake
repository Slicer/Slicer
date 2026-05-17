set(proj python-SimpleITK)

# Set dependency list
set(${proj}_DEPENDENCIES
  python
  python-pip
  python-setuptools
  python-numpy
  )

if(NOT DEFINED Slicer_USE_SYSTEM_${proj})
  set(Slicer_USE_SYSTEM_${proj} ${Slicer_USE_SYSTEM_python})
endif()

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

if(Slicer_USE_SYSTEM_${proj})
  ExternalProject_FindPythonPackage(
    MODULE_NAME "SimpleITK"
    REQUIRED
    )
endif()

if(NOT Slicer_USE_SYSTEM_${proj})
  set(requirements_file ${CMAKE_BINARY_DIR}/${proj}-requirements.txt)
  file(WRITE ${requirements_file} [===[
  # [simpleitk]
  # Hashes correspond to the following packages:
  #  - simpleitk-2.5.5-cp311-abi3-macosx_10_9_x86_64.whl
  #  - simpleitk-2.5.5-cp311-abi3-macosx_11_0_arm64.whl
  #  - simpleitk-2.5.5-cp311-abi3-manylinux2014_aarch64.manylinux_2_17_aarch64.whl
  #  - simpleitk-2.5.5-cp311-abi3-manylinux2014_x86_64.manylinux_2_17_x86_64.whl
  #  - simpleitk-2.5.5-cp311-abi3-win_amd64.whl
  simpleitk==2.5.5 --hash=sha256:e831d15f5ea5b0e02ed3a6ac5f2dd34ba0a0eeaa4ebacfb1d970999836b19ee0 \
                   --hash=sha256:ee87416622cec6ed6b96747a0648c82c6e88f10b6553668ce90736f09ab5a994 \
                   --hash=sha256:847f42057d7bf01b5721b107dd3e00d4ee8d514f6f1ff85794ffedacaf4afc53 \
                   --hash=sha256:8ace5e392be00d5bec28c87ef1d73016f8ebe671dafffe9e9d045f20b0968033 \
                   --hash=sha256:a5fcfcfe9242d3d509b254b3213ca0f5db2c15903f2fc375d9ac5d38e57d415d
  # [/simpleitk]
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

  set(${proj}_PYTHONPATH_LAUNCHER_BUILD
    ${python_DIR}/${PYTHON_SITE_PACKAGES_SUBDIR}
    )
  mark_as_superbuild(
    VARS ${proj}_PYTHONPATH_LAUNCHER_BUILD
    LABELS "PYTHONPATH_LAUNCHER_BUILD"
    )

else()
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
endif()
