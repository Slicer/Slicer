
if(NOT WIN32)

  set(proj tk)

  # Set dependency list
  set(${proj}_DEPENDENCIES tcl)

  if(NOT DEFINED Slicer_USE_SYSTEM_tk)
    set(Slicer_USE_SYSTEM_tk ${Slicer_USE_SYSTEM_tcl})
  endif()

  # Include dependent projects if any
  ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

  if(NOT Slicer_USE_SYSTEM_tk)

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
    if(CMAKE_SIZEOF_VOID_P EQUAL 8) # 64-bit
      set(_configure_cflags "-fPIC")
    endif()

    set(_configure_extra_args)
    if(APPLE)
      set(x_libraries /usr/X11R6/lib)
      set(x_includes /usr/X11R6/include)
      if(DARWIN_MAJOR_VERSION GREATER 11)
        # With version of MacOSX > 11.x (Lion), Apple has dropped dedicated support
        # for X11.app, with users directed to the open source XQuartz project instead.
        # See http://en.wikipedia.org/wiki/XQuartz
        # Headers and libraries provided by XQuartz are located in '/opt/X11'
        set(x_libraries /opt/X11/lib)
        set(x_includes /opt/X11/include)
      endif()
      set(_configure_extra_args --disable-corefoundation --x-libraries=${x_libraries} --x-includes=${x_includes} --with-x)
    endif()

    # configure step
    set(_configure_script ${CMAKE_BINARY_DIR}/${proj}_configure_step.cmake)
    file(WRITE ${_configure_script}
"include(\"${_env_script}\")
set(${proj}_WORKING_DIR \"${tcl_base}/tk/unix\")
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
      URL "https://github.com/Slicer/SlicerBinaryDependencies/releases/download/tk/tk8.6.1-src.tar.gz"
      URL_MD5 "63f21c3a0e0cefbd854b4eb29b129ac6"
      UPDATE_COMMAND "" # Disable update
      SOURCE_DIR ${tcl_base}/tk
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
    # Extra steps to work-around tk build system issues
    if(APPLE)
      foreach(var tcl_build TCL_TK_VERSION_DOT)
        if(NOT DEFINED ${var})
          message(FATAL_ERROR "error: ${var} is not defined !")
        endif()
      endforeach()
      set(_tk_library ${tcl_build}/lib/libtk${TCL_TK_VERSION_DOT}.dylib)
      # XXX Since fixup_bundle expects the library to be writable, let's add an
      # extra step to make sure it's the case.
      ExternalProject_Add_Step(${proj} tk_install_chmod_library
        COMMAND chmod u+xw ${_tk_library}
        DEPENDEES install
        )
      # XXX Tk build system has a known problem building the tk shared library.
      # To ensure fixup_bundle properly process the _tkinter.so python module,
      # we implemented the following workaround to fix the library id.
      # For more details
      #   http://sourceforge.net/p/tcl/mailman/message/30354096/
      #   http://na-mic.org/Mantis/view.php?id=3822
      ExternalProject_Add_Step(${proj} tk_install_fix_library_id
        COMMAND install_name_tool -id ${_tk_library} ${_tk_library}
        DEPENDEES tk_install_chmod_library
        )
    endif()

  else()
    ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
  endif()
endif()

