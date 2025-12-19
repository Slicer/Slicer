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
  #  - simpleitk-2.5.2-cp311-abi3-macosx_10_9_x86_64.whl
  #  - simpleitk-2.5.2-cp311-abi3-macosx_11_0_arm64.whl
  #  - simpleitk-2.5.2-cp311-abi3-manylinux2014_aarch64.manylinux_2_17_aarch64.whl
  #  - simpleitk-2.5.2-cp311-abi3-manylinux2014_x86_64.manylinux_2_17_x86_64.whl
  #  - simpleitk-2.5.2-cp311-abi3-win_amd64.whl
  simpleitk==2.5.2 --hash=sha256:c0227e1365472dd2bd60ea265d016bb82c5b245ad6ab3fcf56ab9be052e3fd22 \
                   --hash=sha256:d91b602a13554ae0a1961076166bd0243c7a0ccbd9c1589ba5afbe8554409371 \
                   --hash=sha256:ce7ef67d22f15632c7f2bbec9d5b40fb18889289df4d04337e2b4021e7e5b444 \
                   --hash=sha256:4a673bdb4ccb7609905f4700fd242591c2273b2ba1604490d79d530661210942 \
                   --hash=sha256:c3884c377c2c5053e15460c6f724a8efe71f4c4ff780418030711b1234114036
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
