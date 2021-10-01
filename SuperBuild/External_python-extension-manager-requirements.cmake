set(proj python-extension-manager-requirements)

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
  foreach(module_name IN ITEMS
    chardet
    couchdb
    git
    gitdb
    six
    smmap
    )
    ExternalProject_FindPythonPackage(
      MODULE_NAME "${module_name}"
      REQUIRED
      )
  endforeach()
endif()

if(NOT Slicer_USE_SYSTEM_${proj})
  set(requirements_file ${CMAKE_BINARY_DIR}/${proj}-requirements.txt)
  file(WRITE ${requirements_file} [===[
  # [chardet]
  chardet==4.0.0 --hash=sha256:f864054d66fd9118f2e67044ac8981a54775ec5b67aed0441892edb553d21da5
  # [/chardet]
  # [CouchDB]
  couchdb==1.2 --hash=sha256:13a28a1159c49f8346732e8724b9a4d65cba54bec017c4a7eeb1499fe88151d1
  # [/CouchDB]
  # [gitdb]
  gitdb==4.0.7 --hash=sha256:6c4cc71933456991da20917998acbe6cf4fb41eeaab7d6d67fbc05ecd4c865b0
  # [/gitdb]
  # [smmap]
  smmap==4.0.0 --hash=sha256:a9a7479e4c572e2e775c404dcd3080c8dc49f39918c2cf74913d30c4c478e3c2
  # [/smmap]
  # [typing-extensions]
  typing-extensions==3.10.0.2 --hash=sha256:f1d25edafde516b146ecd0613dabcc61409817af4766fbbcfb8d1ad4ec441a34
  # [/typing-extensions]
  # [GitPython]
  GitPython==3.1.18 --hash=sha256:fce760879cd2aebd2991b3542876dc5c4a909b30c9d69dfc488e504a8db37ee8
  # [/GitPython]
  # [six]
  six==1.16.0 --hash=sha256:8abb2f1d86890a2dfb989f9a77cfcfd3e47c2a354b01111771326f8aa26e0254
  # [/six]
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

