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
  chardet==5.2.0 --hash=sha256:e1cf59446890a00105fe7b7912492ea04b6e6f06d4b742b2c788469e34c82970
  # [/chardet]
  # [CouchDB]
  couchdb==1.2 --hash=sha256:13a28a1159c49f8346732e8724b9a4d65cba54bec017c4a7eeb1499fe88151d1
  # [/CouchDB]
  # [gitdb]
  gitdb==4.0.12 --hash=sha256:67073e15955400952c6565cc3e707c554a4eea2e428946f7a4c162fab9bd9bcf
  # [/gitdb]
  # [smmap]
  smmap==5.0.2 --hash=sha256:b30115f0def7d7531d22a0fb6502488d879e75b260a9db4d0819cfb25403af5e
  # [/smmap]
  # [GitPython]
  GitPython==3.1.44 --hash=sha256:9e0e10cda9bed1ee64bc9a6de50e7e38a9c9943241cd7f585f6df3ed28011110
  # [/GitPython]
  # [six]
  six==1.17.0 --hash=sha256:4721f391ed90541fddacab5acf947aa0d3dc7d27b2e1e8eda2be8970586c3274
  # [/six]
  ]===])

  ExternalProject_Add(${proj}
    ${${proj}_EP_ARGS}
    DOWNLOAD_COMMAND ""
    SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ${Python3_EXECUTABLE} -m pip install --require-hashes -r ${requirements_file}
    LOG_INSTALL 1
    DEPENDS
      ${${proj}_DEPENDENCIES}
    )

else()
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
endif()
