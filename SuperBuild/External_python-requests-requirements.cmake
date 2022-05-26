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
  certifi==2022.6.15 --hash=sha256:fe86415d55e84719d75f8b69414f6438ac3547d2078ab91b67e779ef69378412
  # [/certifi]
  # [idna]
  idna==3.3 --hash=sha256:84d9dd047ffa80596e0f246e2eab0b391788b0503584e8945f2368256d2735ff
  # [/idna]
  # [charset-normalizer]
  charset-normalizer==2.1.0 --hash=sha256:5189b6f22b01957427f35b6a08d9a0bc45b46d3788ef5a92e978433c7a35f8a5
  # [/charset-normalizer]
  # [urllib3]
  urllib3==1.26.10 --hash=sha256:8298d6d56d39be0e3bc13c1c97d133f9b45d797169a0e11cdd0e0489d786f7ec
  # [/urllib3]
  # [requests]
  requests==2.28.1 --hash=sha256:8fefa2a1a1365bf5520aac41836fbee479da67864514bdb821f31ce07ce65349
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

