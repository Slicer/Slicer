
set(proj tcl)

# Set dependency list
set(${proj}_DEPENDENCIES "")

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

if(${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})
  unset(TCL_LIBRARY CACHE)
  unset(TCL_INCLUDE_PATH CACHE)
  unset(TK_LIBRARY CACHE)
  unset(TK_INCLUDE_PATH CACHE)
  find_package(TCL 8.5 REQUIRED)

  set(TCL_TK_VERSION_DOT "8.5")
  set(TCL_TK_VERSION "85")
endif()

if(NOT ${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})

  set(tcl_SVN_REPOSITORY)
  set(tcl_SOURCE_DIR "")
  set(tcl_BINARY_DIR "")
  set(tcl_BUILD_IN_SOURCE 0)
  set(tcl_CONFIGURE_COMMAND ${CMAKE_COMMAND} -E echo Configuring tcl)
  set(tcl_BUILD_COMMAND ${CMAKE_COMMAND} -E echo Building tcl)
  set(tcl_INSTALL_COMMAND ${CMAKE_COMMAND} -E echo Installing tcl)
  set(tcl_base ${CMAKE_CURRENT_BINARY_DIR}/tcl)
  set(tcl_build ${CMAKE_CURRENT_BINARY_DIR}/tcl-build)

  set(tcl_DOWNLOAD_COMMAND)

  if(WIN32)
    if("${CMAKE_SIZEOF_VOID_P}" EQUAL 8)
      set(TCL_TK_VERSION_DOT "8.5")
      set(TCL_TK_VERSION "85")
      set(INCR_TCL_VERSION_DOT "3.4")
      set(INCR_TCL_VERSION "34")
      set(tcl_SVN_REPOSITORY "http://svn.slicer.org/Slicer3-lib-mirrors/trunk/Binaries/Windows/tcl85-x64-build")
      set(tcl_SVN_REVISION -r "184")
    else()
      set(TCL_TK_VERSION_DOT "8.4")
      set(TCL_TK_VERSION "84")
      set(INCR_TCL_VERSION_DOT "3.2")
      set(INCR_TCL_VERSION "32")
      set(tcl_SVN_REPOSITORY "http://svn.slicer.org/Slicer3-lib-mirrors/trunk/Binaries/Windows/tcl-build")
      set(tcl_SVN_REVISION -r "176")
    endif()
    set(tcl_DOWNLOAD_COMMAND
      SVN_REPOSITORY ${tcl_SVN_REPOSITORY}
      SVN_REVISION ${tcl_SVN_REVISION}
      )
    set(tcl_SOURCE_DIR tcl-build)
    mark_as_superbuild(
      INCR_TCL_VERSION_DOT:STRING
      INCR_TCL_VERSION:STRING
      )
  else()
    set(TCL_TK_VERSION_DOT "8.6")
    set(TCL_TK_VERSION "86")
    set(tcl_SOURCE_DIR tcl/tcl)
    set(tcl_BUILD_IN_SOURCE 1)

    set(tcl_DOWNLOAD_COMMAND
      URL "http://slicer.kitware.com/midas3/download/item/155630/tcl8.6.1-src.tar.gz"
      URL_MD5 "aae4b701ee527c6e4e1a6f9c7399882e"
      )

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

    # configure step
    set(_configure_script ${CMAKE_BINARY_DIR}/${proj}_configure_step.cmake)
    file(WRITE ${_configure_script}
"include(\"${_env_script}\")
set(${proj}_WORKING_DIR \"${tcl_base}/tcl/unix\")
ExternalProject_Execute(${proj} \"configure\" sh configure
  --prefix=${tcl_build} --with-cflags=${_configure_cflags}
  )
")
    set(tcl_CONFIGURE_COMMAND ${CMAKE_COMMAND} -P ${_configure_script})

    # build step
    set(_build_script ${CMAKE_BINARY_DIR}/${proj}_build_step.cmake)
    file(WRITE ${_build_script}
"include(\"${_env_script}\")
set(${proj}_WORKING_DIR \"${tcl_base}/tcl/unix\")
ExternalProject_Execute(${proj} \"build\" make)
")
    set(tcl_BUILD_COMMAND ${CMAKE_COMMAND} -P ${_build_script})

    # install step
    set(_install_script ${CMAKE_BINARY_DIR}/${proj}_install_step.cmake)
    file(WRITE ${_install_script}
"include(\"${_env_script}\")
set(${proj}_WORKING_DIR \"${tcl_base}/tcl/unix\")
ExternalProject_Execute(${proj} \"install\" make install)
")
    set(tcl_INSTALL_COMMAND ${CMAKE_COMMAND} -P ${_install_script})
  endif()

  ExternalProject_Add(${proj}
    ${${proj}_EP_ARGS}
    ${tcl_DOWNLOAD_COMMAND}
    UPDATE_COMMAND "" # Disable update
    SOURCE_DIR ${tcl_SOURCE_DIR}
    BUILD_IN_SOURCE ${tcl_BUILD_IN_SOURCE}
    CONFIGURE_COMMAND ${tcl_CONFIGURE_COMMAND}
    BUILD_COMMAND ${tcl_BUILD_COMMAND}
    INSTALL_COMMAND ${tcl_INSTALL_COMMAND}
    DEPENDS
      ${${proj}_DEPENDENCIES}
    )

  #-----------------------------------------------------------------------------
  # Since fixup_bundle expects the library to be writable, let's add an extra step
  # to make sure it's the case.
  if(APPLE)
    ExternalProject_Add_Step(${proj} tcl_install_chmod_library
      COMMAND chmod u+xw ${tcl_build}/lib/libtcl${TCL_TK_VERSION_DOT}.dylib
      DEPENDEES install
      )
  endif()

  if(WIN32)
    set(TCL_LIBRARY ${tcl_build}/lib/tcl${TCL_TK_VERSION}.lib)
    set(TK_LIBRARY ${tcl_build}/lib/tk${TCL_TK_VERSION}.lib)
  else()
    set(TCL_LIBRARY ${tcl_build}/lib/libtcl${TCL_TK_VERSION_DOT}${CMAKE_SHARED_LIBRARY_SUFFIX})
    set(TK_LIBRARY ${tcl_build}/lib/libtk${TCL_TK_VERSION_DOT}${CMAKE_SHARED_LIBRARY_SUFFIX})
  endif()

  set(Slicer_TCL_DIR ${tcl_build})

else()
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
endif()

mark_as_superbuild(
  Slicer_TCL_DIR:PATH
  TCL_TK_VERSION_DOT:STRING
  TCL_TK_VERSION:STRING
  )


ExternalProject_Message(${proj} "TCL_LIBRARY:${TCL_LIBRARY}")
ExternalProject_Message(${proj} "TK_LIBRARY:${TK_LIBRARY}")
