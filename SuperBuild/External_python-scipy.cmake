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
  #  - scipy-1.9.2-cp39-cp39-macosx_10_9_x86_64.whl
  #  - scipy-1.9.2-cp39-cp39-macosx_12_0_arm64.whl
  #  - scipy-1.9.2-cp39-cp39-manylinux_2_17_aarch64.manylinux2014_aarch64.whl
  #  - scipy-1.9.2-cp39-cp39-manylinux_2_17_x86_64.manylinux2014_x86_64.whl
  #  - scipy-1.9.2-cp39-cp39-win_amd64.whl
  scipy==1.9.2 --hash=sha256:1e3b23a82867018cd26255dc951789a7c567921622073e1113755866f1eae928 \
               --hash=sha256:82e8bfb352aa9dce9a0ffe81f4c369a2c87c85533519441686f59f21d8c09697 \
               --hash=sha256:61b95283529712101bfb7c87faf94cb86ed9e64de079509edfe107e5cfa55733 \
               --hash=sha256:8c8c29703202c39d699b0d6b164bde5501c212005f20abf46ae322b9307c8a41 \
               --hash=sha256:7b2608b3141c257d01ae772e23b3de9e04d27344e6b68a890883795229cb7191
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

