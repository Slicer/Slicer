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
  #  - scipy-1.8.0-cp39-cp39-macosx_10_9_x86_64.whl
  #  - scipy-1.8.0-cp39-cp39-macosx_12_0_arm64.whl
  #  - scipy-1.8.0-cp39-cp39-macosx_12_0_universal2.macosx_10_9_x86_64.whl
  #  - scipy-1.8.0-cp39-cp39-manylinux_2_17_aarch64.manylinux2014_aarch64.whl
  #  - scipy-1.8.0-cp39-cp39-manylinux_2_17_x86_64.manylinux2014_x86_64.whl
  #  - scipy-1.8.0-cp39-cp39-win_amd64.whl
  scipy==1.8.0 --hash=sha256:de2e80ee1d925984c2504812a310841c241791c5279352be4707cdcd7c255039 \
               --hash=sha256:c2bae431d127bf0b1da81fc24e4bba0a84d058e3a96b9dd6475dfcb3c5e8761e \
               --hash=sha256:723b9f878095ed994756fa4ee3060c450e2db0139c5ba248ee3f9628bd64e735 \
               --hash=sha256:e6f0cd9c0bd374ef834ee1e0f0999678d49dcc400ea6209113d81528958f97c7 \
               --hash=sha256:f3720d0124aced49f6f2198a6900304411dbbeed12f56951d7c66ebef05e3df6 \
               --hash=sha256:bb7088e89cd751acf66195d2f00cf009a1ea113f3019664032d9075b1e727b6c
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

