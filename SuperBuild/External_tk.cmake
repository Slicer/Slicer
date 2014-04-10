
if(NOT WIN32)

  set(proj tk)

  # Set dependency list
  set(${proj}_DEPENDENCIES tcl)

  if(NOT DEFINED ${CMAKE_PROJECT_NAME}_USE_SYSTEM_tk)
    set(${CMAKE_PROJECT_NAME}_USE_SYSTEM_tk ${${CMAKE_PROJECT_NAME}_USE_SYSTEM_tcl})
  endif()

  # Include dependent projects if any
  ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

  if(NOT ${CMAKE_PROJECT_NAME}_USE_SYSTEM_tk)

    include(ExternalProjectForNonCMakeProject)

    # environment
    set(_env_script ${CMAKE_BINARY_DIR}/${proj}_Env.cmake)
    ExternalProject_Write_SetBuildEnv_Commands(${_env_script})

    set(_configure_cflags)
    #
    # To fix compilation problem: relocation R_X86_64_32 against `a local symbol' can not be
    # used when making a shared object; recompile with -fPIC
    # See http://www.cmake.org/pipermail/cmake/2007-May/014350.html
    #
    if( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )
      set(_configure_cflags "-fPIC")
    endif()

    set(_configure_extra_args)
    if(APPLE)
      set(_configure_extra_args --disable-corefoundation --x-libraries=/usr/X11R6/lib --x-includes=/usr/X11R6/include --with-x)
    endif()

    # configure step
    set(_configure_script ${CMAKE_BINARY_DIR}/${proj}_configure_step.cmake)
    file(WRITE ${_configure_script}
"include(\"${_env_script}\")
set(${proj}_WORKING_DIR \"${tcl_base}/tk/unix\")
ExternalProject_Remove_Execute_Logs(${proj} \"configure;build;install\")
ExternalProject_Execute(${proj} \"configure\" sh configure
  --with-tcl=${tcl_build}/lib --prefix=${tcl_build} ${_configure_extra_args} --with-cflags=${_configure_cflags}
  )
")

    # build step
    set(_build_script ${CMAKE_BINARY_DIR}/${proj}_build_step.cmake)
    file(WRITE ${_build_script}
"include(\"${_env_script}\")
set(${proj}_WORKING_DIR \"${tcl_base}/tk/unix\")
ExternalProject_Execute(${proj} \"build\" make)
")

    # install step
    set(_install_script ${CMAKE_BINARY_DIR}/${proj}_install_step.cmake)
    file(WRITE ${_install_script}
"include(\"${_env_script}\")
set(${proj}_WORKING_DIR \"${tcl_base}/tk/unix\")
ExternalProject_Execute(${proj} \"install\" make install)
")

    ExternalProject_Add(${proj}
      ${${proj}_EP_ARGS}
      SVN_REPOSITORY "http://svn.slicer.org/Slicer3-lib-mirrors/trunk/tcl/tk"
      SVN_REVISION -r "114"
      UPDATE_COMMAND "" # Disable update
      SOURCE_DIR tcl/tk
      BUILD_IN_SOURCE 1
      CONFIGURE_COMMAND ${CMAKE_COMMAND} -P ${_configure_script}
      BUILD_COMMAND ${CMAKE_COMMAND} -P ${_build_script}
      INSTALL_COMMAND ${CMAKE_COMMAND} -P ${_install_script}
      DEPENDS
        ${${proj}_DEPENDENCIES}
      )

    ExternalProject_Add_Step(${proj} Install_default.h
      COMMAND ${CMAKE_COMMAND} -E copy ${tcl_base}/tk/generic/default.h ${tcl_build}/include
      DEPENDEES install
      )

    ExternalProject_Add_Step(${proj} Install_tkUnixDefault.h
      COMMAND ${CMAKE_COMMAND} -E copy ${tcl_base}/tk/unix/tkUnixDefault.h ${tcl_build}/include
      DEPENDEES install
      )

    #-----------------------------------------------------------------------------
    # Since fixup_bundle expects the library to be writable, let's add an extra step
    # to make sure it's the case.
    if(APPLE)
      foreach(var tcl_build TCL_TK_VERSION_DOT)
        if(NOT DEFINED ${var})
          message(FATAL_ERROR "error: ${var} is not defined !")
        endif()
      endforeach()
      ExternalProject_Add_Step(${proj} tk_install_chmod_library
        COMMAND chmod u+xw ${tcl_build}/lib/libtk${TCL_TK_VERSION_DOT}.dylib
        DEPENDEES install
        )
    endif()

  else()
    ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
  endif()
endif()

