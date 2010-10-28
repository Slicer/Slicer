
#-----------------------------------------------------------------------------
set(tcl_build)
set(proj tcl)

set(tcl_SVN_REPOSITORY)
set(tcl_SOURCE_DIR "")
set(tcl_BINARY_DIR "")
set(tcl_BUILD_IN_SOURCE 0)
set(tcl_CONFIGURE_COMMAND ${CMAKE_COMMAND} -E echo configuring tcl)
set(tcl_BUILD_COMMAND ${CMAKE_COMMAND} -E echo buliding tcl)
set(tcl_INSTALL_COMMAND ${CMAKE_COMMAND} -E echo installing tcl)
set(tcl_base ${CMAKE_CURRENT_BINARY_DIR}/tcl)
set(tcl_build ${CMAKE_CURRENT_BINARY_DIR}/tcl-build)


if(WIN32)
  set(tcl_SVN_REPOSITORY "http://svn.slicer.org/Slicer3-lib-mirrors/trunk/Binaries/Windows/tcl-build")
  set(tcl_SOURCE_DIR tcl-build)
else()
  set(tcl_SVN_REPOSITORY "http://svn.slicer.org/Slicer3-lib-mirrors/trunk/tcl/tcl")
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
  SOURCE_DIR ${tcl_SOURCE_DIR}
  BUILD_IN_SOURCE ${tcl_BUILD_IN_SOURCE}
  UPDATE_COMMAND ""
  CONFIGURE_COMMAND ${tcl_CONFIGURE_COMMAND}
  BUILD_COMMAND ${tcl_BUILD_COMMAND}
  INSTALL_COMMAND ${tcl_INSTALL_COMMAND}
  DEPENDS
    ${tcl_DEPENDENCIES}
)
