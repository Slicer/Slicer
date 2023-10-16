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
  #  - scipy-1.11.3-cp39-cp39-macosx_10_9_x86_64.whl
  #  - scipy-1.11.3-cp39-cp39-macosx_12_0_arm64.whl
  #  - scipy-1.11.3-cp39-cp39-manylinux_2_17_aarch64.manylinux2014_aarch64.whl
  #  - scipy-1.11.3-cp39-cp39-manylinux_2_17_x86_64.manylinux2014_x86_64.whl
  #  - scipy-1.11.3-cp39-cp39-musllinux_1_1_x86_64.whl
  #  - scipy-1.11.3-cp39-cp39-win_amd64.whl
  scipy==1.11.3 --hash=sha256:a63d1ec9cadecce838467ce0631c17c15c7197ae61e49429434ba01d618caa83 \
                --hash=sha256:5305792c7110e32ff155aed0df46aa60a60fc6e52cd4ee02cdeb67eaccd5356e \
                --hash=sha256:9ea7f579182d83d00fed0e5c11a4aa5ffe01460444219dedc448a36adf0c3917 \
                --hash=sha256:c77da50c9a91e23beb63c2a711ef9e9ca9a2060442757dffee34ea41847d8156 \
                --hash=sha256:15f237e890c24aef6891c7d008f9ff7e758c6ef39a2b5df264650eb7900403c0 \
                --hash=sha256:4b4bb134c7aa457e26cc6ea482b016fef45db71417d55cc6d8f43d799cdf9ef2
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

