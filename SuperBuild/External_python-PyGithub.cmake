set(proj python-PyGithub)

# Set dependency list
set(${proj}_DEPENDENCIES python python-setuptools python-pyjwt)

if(NOT DEFINED Slicer_USE_SYSTEM_${proj})
  set(Slicer_USE_SYSTEM_${proj} ${Slicer_USE_SYSTEM_python})
endif()

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

if(Slicer_USE_SYSTEM_${proj})
  ExternalProject_FindPythonPackage(
    MODULE_NAME "github"
    NO_VERSION_PROPERTY
    REQUIRED
    )
endif()

if(NOT Slicer_USE_SYSTEM_${proj})

  set(_version "1.39")

  ExternalProject_Add(${proj}
    ${${proj}_EP_ARGS}
    URL "https://pypi.python.org/packages/fd/28/c42bdd56111dae8c622c01af69e6d7f3d3ff0a96ad846f221749701f86b1/PyGithub-${_version}.tar.gz"
    URL_HASH "SHA256=8a87bc0fbd0b70c2f12911f7f25a493cd13371bc1bbac6c548cc61b69e7d006f"
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

  # See #3749 - Delete test files causing packaging to fail on windows
  ExternalProject_PythonModule_InstallTreeCleanup(${proj} "github" "tests")

else()
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
endif()
