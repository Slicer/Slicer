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
  #  - scipy-1.12.0-cp39-cp39-macosx_10_9_x86_64.whl
  #  - scipy-1.12.0-cp39-cp39-macosx_12_0_arm64.whl
  #  - scipy-1.12.0-cp39-cp39-manylinux_2_17_aarch64.manylinux2014_aarch64.whl
  #  - scipy-1.12.0-cp39-cp39-manylinux_2_17_x86_64.manylinux2014_x86_64.whl
  #  - scipy-1.12.0-cp39-cp39-musllinux_1_1_x86_64.whl
  #  - scipy-1.12.0-cp39-cp39-win_amd64.whl
  scipy==1.12.0 --hash=sha256:913d6e7956c3a671de3b05ccb66b11bc293f56bfdef040583a7221d9e22a2e35 \
                --hash=sha256:bba1b0c7256ad75401c73e4b3cf09d1f176e9bd4248f0d3112170fb2ec4db067 \
                --hash=sha256:730badef9b827b368f351eacae2e82da414e13cf8bd5051b4bdfd720271a5371 \
                --hash=sha256:6546dc2c11a9df6926afcbdd8a3edec28566e4e785b915e849348c6dd9f3f490 \
                --hash=sha256:196ebad3a4882081f62a5bf4aeb7326aa34b110e533aab23e4374fcccb0890dc \
                --hash=sha256:b360f1b6b2f742781299514e99ff560d1fe9bd1bff2712894b52abe528d1fd1e
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

else()
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
endif()
