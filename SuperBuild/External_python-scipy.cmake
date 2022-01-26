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
  #  - scipy-1.7.3-1-cp39-cp39-macosx_12_0_arm64.whl
  #  - scipy-1.7.3-cp39-cp39-macosx_10_9_x86_64.whl
  #  - scipy-1.7.3-cp39-cp39-macosx_12_0_arm64.whl
  #  - scipy-1.7.3-cp39-cp39-manylinux_2_17_aarch64.manylinux2014_aarch64.whl
  #  - scipy-1.7.3-cp39-cp39-manylinux_2_17_x86_64.manylinux2014_x86_64.whl
  #  - scipy-1.7.3-cp39-cp39-win_amd64.whl
  scipy==1.7.3 --hash=sha256:b78a35c5c74d336f42f44106174b9851c783184a85a3fe3e68857259b37b9ffb \
               --hash=sha256:eef93a446114ac0193a7b714ce67659db80caf940f3232bad63f4c7a81bc18df \
               --hash=sha256:eb326658f9b73c07081300daba90a8746543b5ea177184daed26528273157294 \
               --hash=sha256:edad1cf5b2ce1912c4d8ddad20e11d333165552aba262c882e28c78bbc09dbf6 \
               --hash=sha256:5d1cc2c19afe3b5a546ede7e6a44ce1ff52e443d12b231823268019f608b9b12 \
               --hash=sha256:3f78181a153fa21c018d346f595edd648344751d7f03ab94b398be2ad083ed3e
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

