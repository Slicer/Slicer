set(proj python-GitPython)

# Set dependency list
set(${proj}_DEPENDENCIES python python-setuptools python-gitdb)

if(NOT DEFINED ${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})
  set(${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj} ${${CMAKE_PROJECT_NAME}_USE_SYSTEM_python})
endif()

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

if(${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})
  ExternalProject_FindPythonPackage(
    MODULE_NAME "git"
    REQUIRED
    )
endif()



if(NOT ${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})

  set(_version "2.1.7")

  ExternalProject_Add(${proj}
    ${${proj}_EP_ARGS}
    URL "https://pypi.python.org/packages/e1/ec/9f2bbc86a0188bf83221212be87fa4b70dc71a9385efcc95a25e9789894e/GitPython-${_version}.tar.gz"
    URL_MD5 "a63fdd2a650694ea23e268fd0c952034"
    DOWNLOAD_DIR ${CMAKE_BINARY_DIR}
    SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ${PYTHON_EXECUTABLE} setup.py install
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
