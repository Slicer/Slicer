set(proj python-extension-manager-ssl-requirements)

# Set dependency list
set(${proj}_DEPENDENCIES python python-setuptools python-requests-requirements python-pip)

set(requirements_file ${CMAKE_BINARY_DIR}/${proj}-requirements.txt)
file(WRITE ${requirements_file} [===[
PyJWT==1.7.1 --hash=sha256:5c6eca3c2940464d106b99ba83b00c6add741c9becaec087fb7ccdefea71350e
six==1.12.0 --hash=sha256:3350809f0555b11f552448330d0b52d5f24c91a322ea4a15ef22629740f3761c
certifi==2019.9.11 --hash=sha256:fd7c7c74727ddcf00e9acd26bba8da604ffec95bf1c2144e67aff7a8b50e6cef
idna==2.8 --hash=sha256:ea8b7f6188e6fa117537c3df7da9fc686d485087abf6ac197f9c46432f7e4a3c
chardet==3.0.4 --hash=sha256:fc323ffcaeaed0e0a02bf4d117757b98aed530d9ed4531e3e15460124c106691
urllib3==1.25.6 --hash=sha256:3de946ffbed6e6746608990594d08faac602528ac7015ac28d33cee6a45b7398
requests==2.22.0 --hash=sha256:9cf5292fcd0f598c671cfc1e0d7d1a7f13bb8085e9a590f48c010551dc6c4b31
wrapt==1.11.2 --hash=sha256:565a021fd19419476b9362b05eeaa094178de64f8361e44468f9e9d7843901e1
Deprecated==1.2.6 --hash=sha256:b07b414c8aac88f60c1d837d21def7e83ba711052e03b3cbaff27972567a8f8d
PyGithub==1.44 --hash=sha256:fd10fc9006fd54080b190c5c863384381905160c8ea8e830c4a3d8219f23193d
]===])

if(NOT DEFINED Slicer_USE_SYSTEM_${proj})
  set(Slicer_USE_SYSTEM_${proj} ${Slicer_USE_SYSTEM_python})
endif()

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

if(Slicer_USE_SYSTEM_${proj})
  foreach(module_name IN ITEMS jwt)
    ExternalProject_FindPythonPackage(
      MODULE_NAME "${module_name}"
      REQUIRED
      )
  endforeach()
  ExternalProject_FindPythonPackage(
    MODULE_NAME github
    NO_VERSION_PROPERTY
    REQUIRED
    )
endif()

if(NOT Slicer_USE_SYSTEM_${proj})

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

