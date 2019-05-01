set(proj python-PyGithub)

# Set dependency list
set(${proj}_DEPENDENCIES python python-setuptools python-pyjwt python-requests)

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

  set(_version "1.40")

  ExternalProject_Add(${proj}
    ${${proj}_EP_ARGS}
    URL "https://files.pythonhosted.org/packages/9c/f7/bfd1e1dbb6b78f1485e76c9e5cd34833e69ba9705f9f941c4dc5eac0f0de/PyGithub-${_version}.tar.gz"
    URL_HASH "SHA256=b9f5cf80197ba0e3848e084de289ffa19ce1c8c279070a58754ad694d364d481"
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
    LICENSE_FILES "COPYING.LESSER"
    )

  # See #3749 - Delete test files causing packaging to fail on windows
  ExternalProject_PythonModule_InstallTreeCleanup(${proj} "github" "tests")

else()
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
endif()
