set(proj python-scipy)

# Set dependency list
set(${proj}_DEPENDENCIES
  python
  python-ensurepip
  python-numpy
  python-pip
  python-setuptools
  python-wheel
  )

if(NOT DEFINED Slicer_USE_SYSTEM_${proj})
  set(Slicer_USE_SYSTEM_${proj} ${Slicer_USE_SYSTEM_python})
endif()

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

if(Slicer_USE_SYSTEM_${proj})
  foreach(module_name IN ITEMS scipy)
    ExternalProject_FindPythonPackage(
      MODULE_NAME "${module_name}"
      REQUIRED
      )
  endforeach()
endif()

if(NOT Slicer_USE_SYSTEM_${proj})
  set(requirements_file ${CMAKE_BINARY_DIR}/${proj}-requirements.txt)
  file(WRITE ${requirements_file} [===[
  # [scipy]
  # Hashes correspond to the following packages:
  #  - scipy-1.10.1-cp39-cp39-macosx_10_9_x86_64.whl
  #  - scipy-1.10.1-cp39-cp39-macosx_12_0_arm64.whl
  #  - scipy-1.10.1-cp39-cp39-manylinux_2_17_aarch64.manylinux2014_aarch64.whl
  #  - scipy-1.10.1-cp39-cp39-manylinux_2_17_x86_64.manylinux2014_x86_64.whl
  #  - scipy-1.10.1-cp39-cp39-win_amd64.whl
  scipy==1.10.1 --hash=sha256:cd9f1027ff30d90618914a64ca9b1a77a431159df0e2a195d8a9e8a04c78abf9 \
                --hash=sha256:79c8e5a6c6ffaf3a2262ef1be1e108a035cf4f05c14df56057b64acc5bebffb6 \
                --hash=sha256:51af417a000d2dbe1ec6c372dfe688e041a7084da4fdd350aeb139bd3fb55353 \
                --hash=sha256:1b4735d6c28aad3cdcf52117e0e91d6b39acd4272f3f5cd9907c24ee931ad601 \
                --hash=sha256:7ff7f37b1bf4417baca958d254e8e2875d0cc23aaadbe65b3d5b3077b0eb23ea
  # [/scipy]
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

else()
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
endif()

