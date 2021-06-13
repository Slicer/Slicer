set(proj python-requests-requirements)

# Set dependency list
set(${proj}_DEPENDENCIES
  python
  python-pip
  python-setuptools
  )

if(NOT DEFINED Slicer_USE_SYSTEM_${proj})
  set(Slicer_USE_SYSTEM_${proj} ${Slicer_USE_SYSTEM_python})
endif()

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

if(Slicer_USE_SYSTEM_${proj})
  foreach(module_name IN ITEMS certifi idna chardet urllib3 requests)
    ExternalProject_FindPythonPackage(
      MODULE_NAME "${module_name}"
      REQUIRED
      )
  endforeach()
endif()

if(NOT Slicer_USE_SYSTEM_${proj})
  set(requirements_file ${CMAKE_BINARY_DIR}/${proj}-requirements.txt)
  file(WRITE ${requirements_file} [===[
  # [certifi]
  certifi==2021.5.30 --hash=sha256:50b1e4f8446b06f41be7dd6338db18e0990601dce795c2b1686458aa7e8fa7d8
  # [/certifi]
  # [idna]
  idna==2.10 --hash=sha256:b97d804b1e9b523befed77c48dacec60e6dcb0b5391d57af6a65a312a90648c0
  # [/idna]
  # [chardet]
  chardet==4.0.0 --hash=sha256:f864054d66fd9118f2e67044ac8981a54775ec5b67aed0441892edb553d21da5
  # [/chardet]
  # [urllib3]
  urllib3==1.26.5 --hash=sha256:753a0374df26658f99d826cfe40394a686d05985786d946fbe4165b5148f5a7c
  # [/urllib3]
  # [requests]
  requests==2.25.1 --hash=sha256:c210084e36a42ae6b9219e00e48287def368a26d03a048ddad7bfee44f75871e
  # [/requests]
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

