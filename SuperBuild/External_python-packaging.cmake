set(proj python-packaging)

# Set dependency list
set(${proj}_DEPENDENCIES python python-setuptools python-pyparsing)

if(NOT DEFINED Slicer_USE_SYSTEM_${proj})
  set(Slicer_USE_SYSTEM_${proj} ${Slicer_USE_SYSTEM_python})
endif()

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

if(Slicer_USE_SYSTEM_${proj})
  ExternalProject_FindPythonPackage(
    MODULE_NAME "packaging"
    REQUIRED
    )
endif()

if(NOT Slicer_USE_SYSTEM_${proj})

  set(_version "19.0")

  ExternalProject_Add(${proj}
    ${${proj}_EP_ARGS}
    URL "https://files.pythonhosted.org/packages/16/51/d72654dbbaa4a4ffbf7cb0ecd7d12222979e0a660bf3f42acc47550bf098/packaging-${_version}.tar.gz"
    URL_HASH "SHA256=0c98a5d0be38ed775798ece1b9727178c4469d9c3b4ada66e8e6b7849f8732af"
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
    LICENSE_FILES "LICENSE.BSD"
    )

else()
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
endif()
