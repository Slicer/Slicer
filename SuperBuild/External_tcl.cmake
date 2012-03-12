
# Make sure this file is included only once
get_filename_component(CMAKE_CURRENT_LIST_FILENAME ${CMAKE_CURRENT_LIST_FILE} NAME_WE)
if(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED)
  return()
endif()
set(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED 1)

# Set dependency list
set(tcl_DEPENDENCIES "")

# Include dependent projects if any
SlicerMacroCheckExternalProjectDependency(tcl)
set(proj tcl)

#message(STATUS "${__indent}Adding project ${proj}")

set(tcl_SVN_REPOSITORY)
set(tcl_SOURCE_DIR "")
set(tcl_BINARY_DIR "")
set(tcl_BUILD_IN_SOURCE 0)
set(tcl_CONFIGURE_COMMAND ${CMAKE_COMMAND} -E echo Configuring tcl)
set(tcl_BUILD_COMMAND ${CMAKE_COMMAND} -E echo Building tcl)
set(tcl_INSTALL_COMMAND ${CMAKE_COMMAND} -E echo Installing tcl)
set(tcl_base ${CMAKE_CURRENT_BINARY_DIR}/tcl)
set(tcl_build ${CMAKE_CURRENT_BINARY_DIR}/tcl-build)

set(TCL_TK_VERSION_DOT "8.4")
set(TCL_TK_VERSION "84")

if(WIN32)
  if("${CMAKE_SIZEOF_VOID_P}" EQUAL 8)
    set(TCL_TK_VERSION_DOT "8.5")
    set(TCL_TK_VERSION "85")
    set(INCR_TCL_VERSION_DOT "3.4")
    set(INCR_TCL_VERSION "34")
    set(tcl_SVN_REPOSITORY "http://svn.slicer.org/Slicer3-lib-mirrors/trunk/Binaries/Windows/tcl85-x64-build")
    set(tcl_SVN_REVISION -r "159")
  else()
    set(INCR_TCL_VERSION_DOT "3.2")
    set(INCR_TCL_VERSION "32")
    set(tcl_SVN_REPOSITORY "http://svn.slicer.org/Slicer3-lib-mirrors/trunk/Binaries/Windows/tcl-build")
    set(tcl_SVN_REVISION -r "176")
  endif()
  set(tcl_SOURCE_DIR tcl-build)
else()
  set(tcl_SVN_REPOSITORY "http://svn.slicer.org/Slicer3-lib-mirrors/trunk/tcl/tcl")
  set(tcl_SVN_REVISION -r "81")
  set(tcl_SOURCE_DIR tcl/tcl)
  set(tcl_BUILD_IN_SOURCE 1)

  # configure, make and make install all need to be executed in tcl/unix. External_Project
  # doesn't provide any way to set the working directory for each step so we do so by
  # configuring a script that has an execute_process command that has the correct working
  # directory
  configure_file(
    SuperBuild/tcl_configure_step.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/tcl_configure_step.cmake
    @ONLY)

  set(tcl_CONFIGURE_COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/tcl_configure_step.cmake)

  configure_file(
    SuperBuild/tcl_make_step.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/tcl_make_step.cmake
    @ONLY)

  set(tcl_BUILD_COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/tcl_make_step.cmake)

  configure_file(
    SuperBuild/tcl_install_step.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/tcl_install_step.cmake
    @ONLY)

  set(tcl_INSTALL_COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/tcl_install_step.cmake)
endif()

ExternalProject_Add(${proj}
  SVN_REPOSITORY ${tcl_SVN_REPOSITORY}
  SVN_REVISION ${tcl_SVN_REVISION}
  SOURCE_DIR ${tcl_SOURCE_DIR}
  BUILD_IN_SOURCE ${tcl_BUILD_IN_SOURCE}
  UPDATE_COMMAND ""
  CONFIGURE_COMMAND ${tcl_CONFIGURE_COMMAND}
  BUILD_COMMAND ${tcl_BUILD_COMMAND}
  INSTALL_COMMAND ${tcl_INSTALL_COMMAND}
  DEPENDS
    ${tcl_DEPENDENCIES}
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

