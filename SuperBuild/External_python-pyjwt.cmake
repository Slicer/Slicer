set(proj python-pyjwt)

# Set dependency list
set(${proj}_DEPENDENCIES python python-setuptools)

if(NOT DEFINED Slicer_USE_SYSTEM_${proj})
  set(Slicer_USE_SYSTEM_${proj} ${Slicer_USE_SYSTEM_python})
endif()

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

if(Slicer_USE_SYSTEM_${proj})
  ExternalProject_FindPythonPackage(
    MODULE_NAME "jwt"
    NO_VERSION_PROPERTY
    REQUIRED
    )
endif()

if(NOT Slicer_USE_SYSTEM_${proj})

  set(_version "1.7.1")

  ExternalProject_Add(${proj}
    ${${proj}_EP_ARGS}
    URL "https://files.pythonhosted.org/packages/2f/38/ff37a24c0243c5f45f5798bd120c0f873eeed073994133c084e1cf13b95c/PyJWT-${_version}.tar.gz"
    URL_HASH "SHA256=8d59a976fb773f3e6a39c85636357c4f0e242707394cadadd9814f5cbaa20e96"
    DOWNLOAD_DIR ${CMAKE_BINARY_DIR}
    SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ${PYTHON_EXECUTABLE} setup.py install
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
