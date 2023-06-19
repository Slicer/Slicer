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
  #  - scipy-1.11.1-cp39-cp39-macosx_10_9_x86_64.whl
  #  - scipy-1.11.1-cp39-cp39-macosx_12_0_arm64.whl
  #  - scipy-1.11.1-cp39-cp39-manylinux_2_17_aarch64.manylinux2014_aarch64.whl
  #  - scipy-1.11.1-cp39-cp39-manylinux_2_17_x86_64.manylinux2014_x86_64.whl
  #  - scipy-1.11.1-cp39-cp39-musllinux_1_1_x86_64.whl
  #  - scipy-1.11.1-cp39-cp39-win_amd64.whl
  scipy==1.11.1 --hash=sha256:39154437654260a52871dfde852adf1b93b1d1bc5dc0ffa70068f16ec0be2624 \
                --hash=sha256:b588311875c58d1acd4ef17c983b9f1ab5391755a47c3d70b6bd503a45bfaf71 \
                --hash=sha256:d51565560565a0307ed06fa0ec4c6f21ff094947d4844d6068ed04400c72d0c3 \
                --hash=sha256:b41a0f322b4eb51b078cb3441e950ad661ede490c3aca66edef66f4b37ab1877 \
                --hash=sha256:396fae3f8c12ad14c5f3eb40499fd06a6fef8393a6baa352a652ecd51e74e029 \
                --hash=sha256:be8c962a821957fdde8c4044efdab7a140c13294997a407eaee777acf63cbf0c
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

