
# Make sure this file is included only once
get_filename_component(CMAKE_CURRENT_LIST_FILENAME ${CMAKE_CURRENT_LIST_FILE} NAME_WE)
if(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED)
  return()
endif()
set(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED 1)

# Set dependency list
set(incrTcl_DEPENDENCIES tcl tk)

# Include dependent projects if any
SlicerMacroCheckExternalProjectDependency(incrTcl)
set(proj incrTcl)

#message(STATUS "${__indent}Adding project ${proj}")

set(INCR_TCL_VERSION_DOT "3.2")
set(INCR_TCL_VERSION "32")
set(incrTcl_SVN_REPOSITORY "http://svn.slicer.org/Slicer3-lib-mirrors/trunk/tcl/incrTcl")
set(incrTcl_SVN_REVISION -r "4")
set(incrTcl_BUILD_IN_SOURCE 1)
set(incrTcl_PATCH_COMMAND "")

if(APPLE)
  set(incrTcl_configure ${tcl_base}/incrTcl/itcl/configure)
  set(incrTcl_configure_find "*.c | *.o | *.obj) \;\;")
  set(incrTcl_configure_replace "*.c | *.o | *.obj | *.dSYM | *.gnoc ) \;\;")

  set(script ${CMAKE_CURRENT_SOURCE_DIR}/CMake/SlicerBlockStringFindReplace.cmake)
  set(in ${incrTcl_configure})
  set(out ${incrTcl_configure})

  set(incrTcl_PATCH_COMMAND ${CMAKE_COMMAND} -Din=${in} -Dout=${out} -Dfind=${incrTcl_configure_find} -Dreplace=${incrTcl_configure_replace} -P ${script})
endif()

configure_file(
  SuperBuild/incrTcl_configure_step.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/incrTcl_configure_step.cmake
  @ONLY)
set(incrTcl_CONFIGURE_COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/incrTcl_configure_step.cmake)

configure_file(
  SuperBuild/incrTcl_make_step.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/incrTcl_make_step.cmake
  @ONLY)
set(incrTcl_BUILD_COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/incrTcl_make_step.cmake)

configure_file(
  SuperBuild/incrTcl_install_step.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/incrTcl_install_step.cmake
  @ONLY)
set(incrTcl_INSTALL_COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/incrTcl_install_step.cmake)


if(NOT WIN32)
  ExternalProject_Add(${proj}
    SVN_REPOSITORY ${incrTcl_SVN_REPOSITORY}
    SVN_REVISION ${incrTcl_SVN_REVISION}
    UPDATE_COMMAND "" # Disable update
    SOURCE_DIR tcl/incrTcl
    BUILD_IN_SOURCE ${incrTcl_BUILD_IN_SOURCE}
    PATCH_COMMAND ${incrTcl_PATCH_COMMAND}
    CONFIGURE_COMMAND ${incrTcl_CONFIGURE_COMMAND}
    BUILD_COMMAND ${incrTcl_BUILD_COMMAND}
    INSTALL_COMMAND ${incrTcl_INSTALL_COMMAND}
    DEPENDS
      ${incrTcl_DEPENDENCIES}
  )

  ExternalProject_Add_Step(${proj} CHMOD_incrTcl_configure
    COMMAND chmod +x ${tcl_base}/incrTcl/configure
    DEPENDEES patch
    DEPENDERS configure
    )
endif()

