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
  gitdb==4.0.5 --hash=sha256:91f36bfb1ab7949b3b40e23736db18231bf7593edada2ba5c3a174a7b23657ac
  # [/gitdb]
  # [smmap]
  smmap==3.0.5 --hash=sha256:7bfcf367828031dc893530a29cb35eb8c8f2d7c8f2d0989354d75d24c8573714
  # [/smmap]
  # [GitPython]
  GitPython==3.1.14 --hash=sha256:3283ae2fba31c913d857e12e5ba5f9a7772bbc064ae2bb09efafa71b0dd4939b
  # [/GitPython]
  # [six]
  six==1.15.0 --hash=sha256:8b74bedcbbbaca38ff6d7491d76f2b06b3592611af620f8426e82dddb04a5ced
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

