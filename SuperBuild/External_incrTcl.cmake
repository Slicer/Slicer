
set(proj incrTcl)

# Set dependency list
set(${proj}_DEPENDENCIES tcl tk)

if(NOT DEFINED ${CMAKE_PROJECT_NAME}_USE_SYSTEM_incrTcl)
  set(${CMAKE_PROJECT_NAME}_USE_SYSTEM_incrTcl ${${CMAKE_PROJECT_NAME}_USE_SYSTEM_tcl})
endif()

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

if(NOT ${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})

  set(INCR_TCL_VERSION_DOT "4.0")
  set(INCR_TCL_VERSION "40")
  set(incrTcl_DOWNLOAD_COMMAND
    URL "http://slicer.kitware.com/midas3/download/item/155632/itcl4.0.1.tar.gz"
    URL_MD5 "8d71d4247b19ad5d6bcbb776590ced55"
    )

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
      ${incrTcl_DOWNLOAD_COMMAND}
      UPDATE_COMMAND "" # Disable update
      SOURCE_DIR tcl/incrTcl
      BUILD_IN_SOURCE 1
      CONFIGURE_COMMAND ${CMAKE_COMMAND} -P ${_configure_script}
      BUILD_COMMAND ${CMAKE_COMMAND} -P ${_build_script}
      INSTALL_COMMAND ${CMAKE_COMMAND} -P ${_install_script}
      DEPENDS
        ${${proj}_DEPENDENCIES}
    )

    ExternalProject_Add_Step(${proj} CHMOD_incrTcl_configure
      COMMAND chmod +x ${tcl_base}/incrTcl/configure
      DEPENDEES patch
      DEPENDERS configure
      )
  endif()

  mark_as_superbuild(
    INCR_TCL_VERSION_DOT:STRING
    INCR_TCL_VERSION:STRING
    )

else()
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
endif()
