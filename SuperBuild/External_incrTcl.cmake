
set(proj incrTcl)

# Set dependency list
set(${proj}_DEPENDENCIES tcl tk)

if(NOT DEFINED Slicer_USE_SYSTEM_incrTcl)
  set(Slicer_USE_SYSTEM_incrTcl ${Slicer_USE_SYSTEM_tcl})
endif()

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

if(NOT Slicer_USE_SYSTEM_${proj})

  set(INCR_TCL_VERSION_DOT "4.0")
  set(INCR_TCL_VERSION "40")

  if(UNIX)

    include(ExternalProjectForNonCMakeProject)

    # environment
    set(_env_script ${CMAKE_BINARY_DIR}/${proj}_Env.cmake)
    ExternalProject_Write_SetBuildEnv_Commands(${_env_script})

    # configure step
    set(_configure_script ${CMAKE_BINARY_DIR}/${proj}_configure_step.cmake)
    file(WRITE ${_configure_script}
"include(\"${_env_script}\")
set(${proj}_WORKING_DIR \"${tcl_base}/incrTcl\")
ExternalProject_Execute(${proj} \"configure\" sh configure
  --with-tcl=${tcl_build}/lib --with-tk=${tcl_build}/lib --prefix=${tcl_build}
  )
")

    # build step
    set(_build_script ${CMAKE_BINARY_DIR}/${proj}_build_step.cmake)
    file(WRITE ${_build_script}
"include(\"${_env_script}\")
set(${proj}_WORKING_DIR \"${tcl_base}/incrTcl\")
ExternalProject_Execute(${proj} \"build\" make)
")

    # install step
    set(_install_script ${CMAKE_BINARY_DIR}/${proj}_install_step.cmake)
    file(WRITE ${_install_script}
"include(\"${_env_script}\")
set(${proj}_WORKING_DIR \"${tcl_base}/incrTcl\")
ExternalProject_Execute(${proj} \"install\" make install)
")

    ExternalProject_Add(${proj}
      ${${proj}_EP_ARGS}
      URL "https://github.com/Slicer/SlicerBinaryDependencies/releases/download/incrTcl/itcl4.0.1.tar.gz"
      URL_MD5 "8d71d4247b19ad5d6bcbb776590ced55"
      DOWNLOAD_DIR ${CMAKE_BINARY_DIR}
      UPDATE_COMMAND "" # Disable update
      SOURCE_DIR ${tcl_base}/incrTcl
      BUILD_IN_SOURCE 1
      CONFIGURE_COMMAND ${CMAKE_COMMAND} -P ${_configure_script}
      BUILD_COMMAND ${CMAKE_COMMAND} -P ${_build_script}
      INSTALL_COMMAND ${CMAKE_COMMAND} -P ${_install_script}
      DEPENDS
        ${${proj}_DEPENDENCIES}
    )

    ExternalProject_GenerateProjectDescription_Step(${proj}
      VERSION ${INCR_TCL_VERSION_DOT}
      LICENSE_FILES "https://github.com/tcltk/itcl/blob/master/license.terms"
      )

    #-----------------------------------------------------------------------------
    # Launcher setting specific to build tree

    # Note: Search locations for TCL packages - space separated list
    set(TCLLIBPATH "${tcl_build}/lib/itcl4.0.1")

    # environment variables
    set(${proj}_ENVVARS_LAUNCHER_BUILD "TCLLIBPATH=${TCLLIBPATH}")
    mark_as_superbuild(
      VARS ${proj}_ENVVARS_LAUNCHER_BUILD
      LABELS "ENVVARS_LAUNCHER_BUILD"
      )

    #-----------------------------------------------------------------------------
    # Launcher setting specific to install tree

    # library paths
    set(${proj}_LIBRARY_PATHS_LAUNCHER_INSTALLED
      <APPLAUNCHER_SETTINGS_DIR>/../lib/TclTk/lib/itcl4.0.1
      )
    mark_as_superbuild(
      VARS ${proj}_LIBRARY_PATHS_LAUNCHER_INSTALLED
      LABELS "LIBRARY_PATHS_LAUNCHER_INSTALLED"
      )

    # Note: Search locations for TCL packages - space separated list
    set(TCLLIBPATH "<APPLAUNCHER_SETTINGS_DIR>/../lib/TclTk/lib/itcl4.0.1")

    # environment variables
    set(${proj}_ENVVARS_LAUNCHER_INSTALLED "TCLLIBPATH=${TCLLIBPATH}")
    mark_as_superbuild(
      VARS ${proj}_ENVVARS_LAUNCHER_INSTALLED
      LABELS "ENVVARS_LAUNCHER_INSTALLED"
      )
  endif()

  mark_as_superbuild(
    INCR_TCL_VERSION_DOT:STRING
    INCR_TCL_VERSION:STRING
    )

else()
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
endif()
