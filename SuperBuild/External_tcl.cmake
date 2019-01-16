
set(proj tcl)

# Set dependency list
set(${proj}_DEPENDENCIES "")

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

if(Slicer_USE_SYSTEM_${proj})
  unset(TCL_LIBRARY CACHE)
  unset(TCL_INCLUDE_PATH CACHE)
  unset(TK_LIBRARY CACHE)
  unset(TK_INCLUDE_PATH CACHE)
  find_package(TCL 8.5 REQUIRED)

  set(TCL_TK_VERSION_DOT "8.5")
  set(TCL_TK_VERSION "85")
endif()

if(NOT Slicer_USE_SYSTEM_${proj})

  set(tcl_SOURCE_DIR "")
  set(tcl_BINARY_DIR "")
  set(tcl_BUILD_IN_SOURCE 0)
  set(tcl_CONFIGURE_COMMAND ${CMAKE_COMMAND} -E echo Configuring tcl)
  set(tcl_BUILD_COMMAND ${CMAKE_COMMAND} -E echo Building tcl)
  set(tcl_INSTALL_COMMAND ${CMAKE_COMMAND} -E echo Installing tcl)
  set(tcl_base ${CMAKE_BINARY_DIR}/tcl)
  set(tcl_build ${CMAKE_BINARY_DIR}/tcl-build)

  set(tcl_DOWNLOAD_COMMAND)
  set(tcl_PATCH_COMMAND)

  if(WIN32)
    set(TCL_TK_VERSION_DOT "8.5")
    set(TCL_TK_VERSION "85")
    set(INCR_TCL_VERSION_DOT "3.4")
    set(INCR_TCL_VERSION "34")
    if("${CMAKE_SIZEOF_VOID_P}" EQUAL 8)
      set(tcl_URL "https://github.com/Slicer/SlicerBinaryDependencies/releases/download/tcl/tcl85-x64-build.zip")
      set(tcl_MD5 "10f0a12590acaaef924dc22e80c9d70a")
    else()
      set(tcl_URL "https://github.com/Slicer/SlicerBinaryDependencies/releases/download/tcl/tcl85-build.zip")
      set(tcl_MD5 "07401cf7128a9a79403c8d9b745024a6")
    endif()
    set(tcl_DOWNLOAD_COMMAND
      URL ${tcl_URL}
      URL_MD5 ${tcl_MD5}
      )
    set(tcl_SOURCE_DIR ${CMAKE_BINARY_DIR}/tcl-build)
    mark_as_superbuild(
      INCR_TCL_VERSION_DOT:STRING
      INCR_TCL_VERSION:STRING
      )

    #-----------------------------------------------------------------------------
    # Launcher setting specific to build tree

    # library paths
    set(${proj}_incrTcl_LIBRARY_PATHS_LAUNCHER_BUILD
      ${tcl_build}/lib/itcl${INCR_TCL_VERSION_DOT}
      ${tcl_build}/lib/itk${INCR_TCL_VERSION_DOT}
      )
    mark_as_superbuild(
      VARS ${proj}_incrTcl_LIBRARY_PATHS_LAUNCHER_BUILD
      LABELS "LIBRARY_PATHS_LAUNCHER_BUILD"
      )

    # Note: Search locations for TCL packages - space separated list
    set(TCLLIBPATH "${tcl_build}/lib/itcl${INCR_TCL_VERSION_DOT}")
    set(TCLLIBPATH "${TCLLIBPATH} ${tcl_build}/lib/itk${INCR_TCL_VERSION_DOT}")

    # environment variables
    set(${proj}_incrTcl_ENVVARS_LAUNCHER_BUILD "TCLLIBPATH=${TCLLIBPATH}")
    mark_as_superbuild(
      VARS ${proj}_incrTcl_ENVVARS_LAUNCHER_BUILD
      LABELS "ENVVARS_LAUNCHER_BUILD"
      )

    #-----------------------------------------------------------------------------
    # Launcher setting specific to install tree

    # library paths
    set(${proj}_incrTcl_LIBRARY_PATHS_LAUNCHER_INSTALLED
      <APPLAUNCHER_SETTINGS_DIR>/../lib/TclTk/lib/itcl${INCR_TCL_VERSION_DOT}
      <APPLAUNCHER_SETTINGS_DIR>/../lib/TclTk/lib/itk${INCR_TCL_VERSION_DOT}
      )
    mark_as_superbuild(
      VARS ${proj}_incrTcl_LIBRARY_PATHS_LAUNCHER_INSTALLED
      LABELS "LIBRARY_PATHS_LAUNCHER_INSTALLED"
      )

    # Note: Search locations for TCL packages - space separated list
    set(TCLLIBPATH "<APPLAUNCHER_SETTINGS_DIR>/../lib/TclTk/lib/itcl${INCR_TCL_VERSION_DOT}")
    set(TCLLIBPATH "${TCLLIBPATH} <APPLAUNCHER_SETTINGS_DIR>/../lib/TclTk/lib/itk${INCR_TCL_VERSION_DOT}")

    # environment variables
    set(${proj}_incrTcl_ENVVARS_LAUNCHER_INSTALLED "TCLLIBPATH=${TCLLIBPATH}")
    mark_as_superbuild(
      VARS ${proj}_incrTcl_ENVVARS_LAUNCHER_INSTALLED
      LABELS "ENVVARS_LAUNCHER_INSTALLED"
      )

  else()
    set(TCL_TK_VERSION_DOT "8.6")
    set(TCL_TK_VERSION "86")
    set(tcl_SOURCE_DIR ${CMAKE_BINARY_DIR}/tcl/tcl)
    set(tcl_BUILD_IN_SOURCE 1)

    set(tcl_DOWNLOAD_COMMAND
      URL "https://github.com/Slicer/SlicerBinaryDependencies/releases/download/tcl/tcl8.6.1-src.tar.gz"
      URL_MD5 "aae4b701ee527c6e4e1a6f9c7399882e"
      )

    include(ExternalProjectForNonCMakeProject)

    # patch: Since (1) a more recent version of itcl is provided by External_incrTcl
    #        and (2) packages bundled with tcl are not used, the patch command
    #        simply remove the 'pkgs' folder.
    set(tcl_PATCH_COMMAND
      PATCH_COMMAND ${CMAKE_COMMAND} -E remove_directory ${tcl_base}/tcl/pkgs
      )

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
    ${tcl_PATCH_COMMAND}
    CONFIGURE_COMMAND ${tcl_CONFIGURE_COMMAND}
    BUILD_COMMAND ${tcl_BUILD_COMMAND}
    INSTALL_COMMAND ${tcl_INSTALL_COMMAND}
    DEPENDS
      ${${proj}_DEPENDENCIES}
    )

  ExternalProject_GenerateProjectDescription_Step(${proj}
    VERSION ${TCL_TK_VERSION_DOT}
    LICENSE_FILES "https://raw.githubusercontent.com/tcltk/tcl/master/license.terms"
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
    set(${proj}_LIBRARY_PATHS_LAUNCHER_BUILD ${tcl_build}/bin)
  else()
    set(TCL_LIBRARY ${tcl_build}/lib/libtcl${TCL_TK_VERSION_DOT}${CMAKE_SHARED_LIBRARY_SUFFIX})
    set(TK_LIBRARY ${tcl_build}/lib/libtk${TCL_TK_VERSION_DOT}${CMAKE_SHARED_LIBRARY_SUFFIX})
    set(${proj}_LIBRARY_PATHS_LAUNCHER_BUILD ${tcl_build}/lib)
  endif()

  set(Slicer_TCL_DIR ${tcl_build})

  #-----------------------------------------------------------------------------
  # Sanity checks

  foreach(varname IN ITEMS
      PYTHON_STDLIB_SUBDIR
      )
    if("${${varname}}" STREQUAL "")
      message(FATAL_ERROR "${varname} CMake variable is expected to be set")
    endif()
  endforeach()

  #-----------------------------------------------------------------------------
  # Launcher setting specific to build tree

  # library paths
  mark_as_superbuild(
    VARS ${proj}_LIBRARY_PATHS_LAUNCHER_BUILD
    LABELS "LIBRARY_PATHS_LAUNCHER_BUILD"
    )

  # paths
  set(${proj}_PATHS_LAUNCHER_BUILD ${Slicer_TCL_DIR}/bin)
  mark_as_superbuild(
    VARS ${proj}_PATHS_LAUNCHER_BUILD
    LABELS "PATHS_LAUNCHER_BUILD"
    )

  set(_pythonhome ${CMAKE_BINARY_DIR}/python-install)

  # pythonpath
  set(${proj}_PYTHONPATH_LAUNCHER_BUILD
    ${_pythonhome}/${PYTHON_STDLIB_SUBDIR}/lib-tk
    )
  mark_as_superbuild(
    VARS ${proj}_PYTHONPATH_LAUNCHER_BUILD
    LABELS "PYTHONPATH_LAUNCHER_BUILD"
    )

  # environment variables
  set(${proj}_ENVVARS_LAUNCHER_BUILD
    "TCL_LIBRARY=${Slicer_TCL_DIR}/lib/tcl${TCL_TK_VERSION_DOT}"
    "TK_LIBRARY=${Slicer_TCL_DIR}/lib/tk${TCL_TK_VERSION_DOT}"
    )
  mark_as_superbuild(
    VARS ${proj}_ENVVARS_LAUNCHER_BUILD
    LABELS "ENVVARS_LAUNCHER_BUILD"
    )

  #-----------------------------------------------------------------------------
  # Launcher setting specific to install tree

  set(tcllib_subdir lib)
  if(WIN32)
    set(tcllib_subdir bin)
  endif()

  # library paths
  set(${proj}_LIBRARY_PATHS_LAUNCHER_INSTALLED <APPLAUNCHER_SETTINGS_DIR>/../lib/TclTk/${tcllib_subdir})
  mark_as_superbuild(
    VARS ${proj}_LIBRARY_PATHS_LAUNCHER_INSTALLED
    LABELS "LIBRARY_PATHS_LAUNCHER_INSTALLED"
    )

  # pythonpath
  set(${proj}_PYTHONPATH_LAUNCHER_INSTALLED
    <APPLAUNCHER_SETTINGS_DIR>/../lib/Python/${PYTHON_STDLIB_SUBDIR}/lib-tk
    )
  mark_as_superbuild(
    VARS ${proj}_PYTHONPATH_LAUNCHER_INSTALLED
    LABELS "PYTHONPATH_LAUNCHER_INSTALLED"
    )

  # environment variables
  set(${proj}_ENVVARS_LAUNCHER_INSTALLED
    "TCL_LIBRARY=<APPLAUNCHER_SETTINGS_DIR>/../lib/TclTk/lib/tcl${TCL_TK_VERSION_DOT}"
    "TK_LIBRARY=<APPLAUNCHER_SETTINGS_DIR>/../lib/TclTk/lib/tk${TCL_TK_VERSION_DOT}"
    )
  mark_as_superbuild(
    VARS ${proj}_ENVVARS_LAUNCHER_INSTALLED
    LABELS "ENVVARS_LAUNCHER_INSTALLED"
    )

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
