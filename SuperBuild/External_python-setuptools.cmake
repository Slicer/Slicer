set(proj python-setuptools)

# Set dependency list
set(${proj}_DEPENDENCIES python python-packaging python-six python-appdirs)

if(NOT DEFINED ${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})
  set(${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj} ${${CMAKE_PROJECT_NAME}_USE_SYSTEM_python})
endif()

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

if(${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})
  ExternalProject_FindPythonPackage(
    MODULE_NAME "setuptools"
    REQUIRED
    )
endif()

if(NOT ${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})

  if(NOT DEFINED git_protocol)
    set(git_protocol "git")
  endif()

  include(ExternalProjectForNonCMakeProject)

  # environment
  set(_env_script ${CMAKE_BINARY_DIR}/${proj}_Env.cmake)
  ExternalProject_Write_SetBuildEnv_Commands(${_env_script})
  ExternalProject_Write_SetPythonSetupEnv_Commands(${_env_script} APPEND)

  # install step
  # - we use "easy_install" only to allow installing with "--always-unzip"
  set(_install_script ${CMAKE_BINARY_DIR}/${proj}_install_step.cmake)
  file(WRITE ${_install_script}
"include(\"${_env_script}\")
set(${proj}_WORKING_DIR \"${CMAKE_BINARY_DIR}/${proj}\")
# If any, remove lock file
set(lock_file \"${python_DIR}/${PYTHON_SITE_PACKAGES_SUBDIR}/easy-install.pth.lock\")
file(REMOVE \${lock_file})
ExternalProject_Execute(${proj} \"bootstrap\" \"${PYTHON_EXECUTABLE}\" bootstrap.py)
ExternalProject_Execute(${proj} \"easy_install\" \"${PYTHON_EXECUTABLE}\" setup.py easy_install --always-unzip .)
")

  ExternalProject_Add(${proj}
    ${${proj}_EP_ARGS}
    # slicer-v35.0.1
    # - include patch to support parallel build
    GIT_REPOSITORY "${git_protocol}://github.com/Slicer/setuptools.git"
    GIT_TAG "057883d7d94f5b2e18282d592b2ba9d7902a1a6e"
    SOURCE_DIR ${proj}
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ${CMAKE_COMMAND} -P ${_install_script}
    DEPENDS
      ${${proj}_DEPENDENCIES}
    )

  ExternalProject_GenerateProjectDescription_Step(${proj}
    SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}
    )

else()
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
endif()
