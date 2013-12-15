
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

    set(tk_SVN_REPOSITORY "http://svn.slicer.org/Slicer3-lib-mirrors/trunk/tcl/tk")
    set(tk_SVN_REVISION -r "114")
    set(tk_SOURCE_DIR "")
    set(tk_BINARY_DIR "")
    set(tk_BUILD_IN_SOURCE 0)
    set(tk_CONFIGURE_COMMAND "")
    set(tk_BUILD_COMMAND "")
    set(tk_INSTALL_COMMAND "")

    set(tk_SOURCE_DIR tcl/tk)
    set(tk_BUILD_IN_SOURCE 1)

    # configure, make and make install all need to be executed in tk/unix. External_Project
    # doesn't provide any way to set the working directory for each step so we do so by
    # configuring a script that has an execute_process command that has the correct working
    # directory
    configure_file(
      SuperBuild/tk_configure_step.cmake.in
      ${CMAKE_CURRENT_BINARY_DIR}/tk_configure_step.cmake
      @ONLY)

    set(tk_CONFIGURE_COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/tk_configure_step.cmake)

    configure_file(
      SuperBuild/tk_make_step.cmake.in
      ${CMAKE_CURRENT_BINARY_DIR}/tk_make_step.cmake
      @ONLY)

    set(tk_BUILD_COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/tk_make_step.cmake)

    configure_file(
      SuperBuild/tk_install_step.cmake.in
      ${CMAKE_CURRENT_BINARY_DIR}/tk_install_step.cmake
      @ONLY)

    set(tk_INSTALL_COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/tk_install_step.cmake)

    ExternalProject_Add(${proj}
      ${${proj}_EP_ARGS}
      SVN_REPOSITORY ${tk_SVN_REPOSITORY}
      SVN_REVISION ${tk_SVN_REVISION}
      UPDATE_COMMAND "" # Disable update
      SOURCE_DIR ${tk_SOURCE_DIR}
      BUILD_IN_SOURCE ${tk_BUILD_IN_SOURCE}
      CONFIGURE_COMMAND ${tk_CONFIGURE_COMMAND}
      BUILD_COMMAND ${tk_BUILD_COMMAND}
      INSTALL_COMMAND ${tk_INSTALL_COMMAND}
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

