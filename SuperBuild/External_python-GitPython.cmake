set(proj python-GitPython)

# Set dependency list
set(${proj}_DEPENDENCIES python python-setuptools python-gitdb)

if(NOT DEFINED Slicer_USE_SYSTEM_${proj})
  set(Slicer_USE_SYSTEM_${proj} ${Slicer_USE_SYSTEM_python})
endif()

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

if(Slicer_USE_SYSTEM_${proj})
  ExternalProject_FindPythonPackage(
    MODULE_NAME "git"
    REQUIRED
    )
endif()



if(NOT Slicer_USE_SYSTEM_${proj})

  set(_version "2.1.11")

  ExternalProject_Add(${proj}
    ${${proj}_EP_ARGS}
    URL "https://files.pythonhosted.org/packages/4d/e8/98e06d3bc954e3c5b34e2a579ddf26255e762d21eb24fede458eff654c51/GitPython-${_version}.tar.gz"
    URL_HASH "SHA256=8237dc5bfd6f1366abeee5624111b9d6879393d84745a507de0fda86043b65a8"
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

  # Set 'GIT_PYTHON_GIT_EXECUTABLE' env. variable to ensure "import git" done
  # in ExternalProject_PythonModule_InstallTreeCleanup succeeds on platform where
  # git is not available in the PATH.
  set(${proj}_EP_PYTHONMODULE_INSTALL_TREE_CLEANUP_CODE_BEFORE_IMPORT "import os
os.environ[\"GIT_PYTHON_GIT_EXECUTABLE\"] = \"${GIT_EXECUTABLE}\"
"
  )
  # See #3749 - Delete test files causing packaging to fail on windows
  ExternalProject_PythonModule_InstallTreeCleanup(${proj} "git" "test")

else()
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
endif()
