set(proj python-pip)

# Set dependency list
set(${proj}_DEPENDENCIES
  python
  python-ensurepip
  python-setuptools
  )

if(NOT DEFINED Slicer_USE_SYSTEM_${proj})
  set(Slicer_USE_SYSTEM_${proj} ${Slicer_USE_SYSTEM_python})
endif()

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

if(Slicer_USE_SYSTEM_${proj})
  foreach(module_name IN ITEMS pip)
    ExternalProject_FindPythonPackage(
      MODULE_NAME "${module_name}"
      REQUIRED
      )
  endforeach()
endif()

if(NOT Slicer_USE_SYSTEM_${proj})
  set(requirements_file ${CMAKE_BINARY_DIR}/${proj}-requirements.txt)
  file(WRITE ${requirements_file} [===[
  # [pip]
  pip==25.1.1 --hash=sha256:2913a38a2abf4ea6b64ab507bd9e967f3b53dc1ede74b01b0931e1ce548751af
  # [/pip]
  ]===])

  ExternalProject_Add(${proj}
    ${${proj}_EP_ARGS}
    DOWNLOAD_COMMAND ""
    SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ${Python3_EXECUTABLE} -m pip install --require-hashes -r ${requirements_file}
    LOG_INSTALL 1
    DEPENDS
      ${${proj}_DEPENDENCIES}
    )

  #-----------------------------------------------------------------------------
  # Slicer Launcher setting specific to build tree

  # environment variables
  set(${proj}_ENVVARS_LAUNCHER_BUILD
    "PIP_DISABLE_PIP_VERSION_CHECK=1"
    )
  mark_as_superbuild(
    VARS ${proj}_ENVVARS_LAUNCHER_BUILD
    LABELS "ENVVARS_LAUNCHER_BUILD"
    )

  #-----------------------------------------------------------------------------
  # Slicer Launcher setting specific to install tree

  # environment variables
  set(${proj}_ENVVARS_LAUNCHER_INSTALLED
    "PIP_DISABLE_PIP_VERSION_CHECK=1"
    )
  mark_as_superbuild(
    VARS ${proj}_ENVVARS_LAUNCHER_INSTALLED
    LABELS "ENVVARS_LAUNCHER_INSTALLED"
    )

else()
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
endif()
