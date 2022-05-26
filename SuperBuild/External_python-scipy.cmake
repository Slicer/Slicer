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
  #  - scipy-1.8.1-cp39-cp39-macosx_10_9_x86_64.whl
  #  - scipy-1.8.1-cp39-cp39-macosx_12_0_arm64.whl
  #  - scipy-1.8.1-cp39-cp39-macosx_12_0_universal2.macosx_10_9_x86_64.whl
  #  - scipy-1.8.1-cp39-cp39-manylinux_2_17_aarch64.manylinux2014_aarch64.whl
  #  - scipy-1.8.1-cp39-cp39-manylinux_2_17_x86_64.manylinux2014_x86_64.whl
  #  - scipy-1.8.1-cp39-cp39-win_amd64.whl
  scipy==1.8.1 --hash=sha256:f3e7a8867f307e3359cc0ed2c63b61a1e33a19080f92fe377bc7d49f646f2ec1 \
               --hash=sha256:2ef0fbc8bcf102c1998c1f16f15befe7cffba90895d6e84861cd6c6a33fb54f6 \
               --hash=sha256:83606129247e7610b58d0e1e93d2c5133959e9cf93555d3c27e536892f1ba1f2 \
               --hash=sha256:d3b3c8924252caaffc54d4a99f1360aeec001e61267595561089f8b5900821bb \
               --hash=sha256:70de2f11bf64ca9921fda018864c78af7147025e467ce9f4a11bc877266900a6 \
               --hash=sha256:9dd4012ac599a1e7eb63c114d1eee1bcfc6dc75a29b589ff0ad0bb3d9412034f
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

