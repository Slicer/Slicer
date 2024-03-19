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
  gitdb==4.0.11 --hash=sha256:81a3407ddd2ee8df444cbacea00e2d038e40150acfa3001696fe0dcf1d3adfa4
  # [/gitdb]
  # [smmap]
  smmap==5.0.1 --hash=sha256:e6d8668fa5f93e706934a62d7b4db19c8d9eb8cf2adbb75ef1b675aa332b69da
  # [/smmap]
  # [GitPython]
  GitPython==3.1.42 --hash=sha256:1bf9cd7c9e7255f77778ea54359e54ac22a72a5b51288c457c881057b7bb9ecd
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

else()
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
endif()
