set(proj python-setuptools)

# Set dependency list
set(${proj}_DEPENDENCIES python)

if(NOT DEFINED Slicer_USE_SYSTEM_${proj})
  set(Slicer_USE_SYSTEM_${proj} ${Slicer_USE_SYSTEM_python})
endif()

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

if(Slicer_USE_SYSTEM_${proj})
  ExternalProject_FindPythonPackage(
    MODULE_NAME "setuptools"
    REQUIRED
    )
endif()

if(NOT Slicer_USE_SYSTEM_${proj})

  include(ExternalProjectForNonCMakeProject)

  # environment
  set(_env_script ${CMAKE_BINARY_DIR}/${proj}_Env.cmake)
  ExternalProject_Write_SetBuildEnv_Commands(${_env_script})
  ExternalProject_Write_SetPythonSetupEnv_Commands(${_env_script} APPEND)

  set(EP_SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj})

  # install step
  # - we use "easy_install" only to allow installing with "--always-unzip"
  set(_install_script ${CMAKE_BINARY_DIR}/${proj}_install_step.cmake)
  file(WRITE ${_install_script}
"include(\"${_env_script}\")
set(${proj}_WORKING_DIR \"${CMAKE_BINARY_DIR}/${proj}\")
ExternalProject_Execute(${proj} \"bootstrap\" \"${PYTHON_EXECUTABLE}\" bootstrap.py)
ExternalProject_Execute(${proj} \"easy_install\" \"${PYTHON_EXECUTABLE}\" setup.py easy_install --always-unzip .)
")

  ExternalProject_Add(${proj}
    ${${proj}_EP_ARGS}
    # slicer-v40.8.0
    # - include patch to support parallel build
    GIT_REPOSITORY "${EP_GIT_PROTOCOL}://github.com/Slicer/setuptools.git"
    GIT_TAG "cb19e30abe623914ad21297ea64838c38c82a2fe"
    DOWNLOAD_DIR ${CMAKE_BINARY_DIR}
    SOURCE_DIR ${EP_SOURCE_DIR}
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ${CMAKE_COMMAND} -P ${_install_script}
    DEPENDS
      ${${proj}_DEPENDENCIES}
    )

  ExternalProject_GenerateProjectDescription_Step(${proj}
    SOURCE_DIR ${EP_SOURCE_DIR}
    )

else()
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
endif()
