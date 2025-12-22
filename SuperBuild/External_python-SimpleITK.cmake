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
  #  - simpleitk-2.5.3-cp311-abi3-macosx_10_9_x86_64.whl
  #  - simpleitk-2.5.3-cp311-abi3-macosx_11_0_arm64.whl
  #  - simpleitk-2.5.3-cp311-abi3-manylinux2014_aarch64.manylinux_2_17_aarch64.whl
  #  - simpleitk-2.5.3-cp311-abi3-manylinux2014_x86_64.manylinux_2_17_x86_64.whl
  #  - simpleitk-2.5.3-cp311-abi3-win_amd64.whl
  simpleitk==2.5.3 --hash=sha256:eda739126d3cdda29266b722c3bb182a534ee4b4b60a6e565c20e1ebbd7ca2da \
                   --hash=sha256:5438fb87b7e3380b1ba02b2447bf9f474560b51a649b2ad6973195c6515a43a5 \
                   --hash=sha256:724a8fba4a493a9da06cbba545521174092927acf99fbc0594f4f257d66061ff \
                   --hash=sha256:b615a96826815471965899d50d089fb67381a4d59b65750eafe58a6a980ecce2 \
                   --hash=sha256:31b187922c53c858f8604b4f90ebd7aae809e680de751674f5f95ccefcf674d5
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
