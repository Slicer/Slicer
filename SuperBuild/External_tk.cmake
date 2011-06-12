
# Make sure this file is included only once
get_filename_component(CMAKE_CURRENT_LIST_FILENAME ${CMAKE_CURRENT_LIST_FILE} NAME_WE)
IF(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED)
  RETURN()
ENDIF()
SET(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED 1)

if(NOT WIN32)

  # Set dependency list
  set(tk_DEPENDENCIES tcl)

  set(proj tk)
  include(${Slicer_SOURCE_DIR}/CMake/SlicerBlockCheckExternalProjectDependencyList.cmake)
  set(${proj}_EXTERNAL_PROJECT_INCLUDED TRUE)

  #message(STATUS "Adding project '${proj}'")
  
  set(tk_SVN_REPOSITORY "http://svn.slicer.org/Slicer3-lib-mirrors/trunk/tcl/tk")
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
    SVN_REPOSITORY ${tk_SVN_REPOSITORY}
    SOURCE_DIR ${tk_SOURCE_DIR}
    UPDATE_COMMAND ""
    BUILD_IN_SOURCE ${tk_BUILD_IN_SOURCE}
    CONFIGURE_COMMAND ${tk_CONFIGURE_COMMAND}
    BUILD_COMMAND ${tk_BUILD_COMMAND}
    INSTALL_COMMAND ${tk_INSTALL_COMMAND}
    DEPENDS
      ${tk_DEPENDENCIES}
    )

  ExternalProject_Add_Step(${proj} Install_default.h
    COMMAND ${CMAKE_COMMAND} -E copy ${tcl_base}/tk/generic/default.h ${tcl_build}/include
    DEPENDEES install
    )

  ExternalProject_Add_Step(${proj} Install_tkUnixDefault.h
    COMMAND ${CMAKE_COMMAND} -E copy ${tcl_base}/tk/unix/tkUnixDefault.h ${tcl_build}/include
    DEPENDEES install
    )
endif()

