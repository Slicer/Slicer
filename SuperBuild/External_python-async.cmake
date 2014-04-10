set(proj python-async)

# Set dependency list
set(${proj}_DEPENDENCIES python python-setuptools)

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

if(${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})
  # XXX - Add a test checking if <proj> is available
endif()

if(NOT DEFINED ${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})
  set(${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj} ${${CMAKE_PROJECT_NAME}_USE_SYSTEM_python})
endif()

if(NOT ${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})

  include(ExternalProjectForNonCMakeProject)

  # environment
  set(_env_script ${CMAKE_BINARY_DIR}/${proj}_Env.cmake)
  ExternalProject_Write_SetBuildEnv_Commands(${_env_script})
  ExternalProject_Write_SetPythonSetupEnv_Commands(${_env_script} APPEND)

  # install step
  set(_install_script ${CMAKE_BINARY_DIR}/${proj}_install_step.cmake)
  file(WRITE ${_install_script}
"include(\"${_env_script}\")
set(${proj}_WORKING_DIR \"${CMAKE_BINARY_DIR}/${proj}\")
ExternalProject_Execute(${proj} \"install\" \"${PYTHON_EXECUTABLE}\" setup.py install)
")

  ExternalProject_Add(${proj}
    ${${proj}_EP_ARGS}
    URL "https://pypi.python.org/packages/source/a/async/async-0.6.1.tar.gz"
    URL_MD5 "6f0e2ced1fe85f8410b9bde11be08587"
    SOURCE_DIR ${proj}
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ${CMAKE_COMMAND} -P ${_install_script}
    DEPENDS
      ${${proj}_DEPENDENCIES}
    )

else()
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
endif()
