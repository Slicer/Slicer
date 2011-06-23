
# Make sure this file is included only once
get_filename_component(CMAKE_CURRENT_LIST_FILENAME ${CMAKE_CURRENT_LIST_FILE} NAME_WE)
if(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED)
  return()
endif()
set(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED 1)

# Set dependency list
set(weave_DEPENDENCIES python NUMPY)

# Include dependent projects if any
SlicerMacroCheckExternalProjectDependency(weave)
set(proj weave)

#message(STATUS "${__indent}Adding project ${proj}")

set(weave_binary "${CMAKE_CURRENT_BINARY_DIR}/weave/")

# to configure weave we run a cmake -P script
# the script will create a site.cfg file
# then run python setup.py config to verify setup
# configure_file(
#   SuperBuild/weave_configure_step.cmake.in
#   ${CMAKE_CURRENT_BINARY_DIR}/weave_configure_step.cmake @ONLY)
# to build weave we also run a cmake -P script.
# the script will set LD_LIBRARY_PATH so that
# python can run after it is built on linux
configure_file(
  SuperBuild/weave_make_step.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/weave_make_step.cmake @ONLY)

# create an external project to download weave,
# and configure and build it
ExternalProject_Add(weave
  # URL ${Slicer_SOURCE_DIR}/Modules/Python/FilteredTractography/weave
  SVN_REPOSITORY http://svn.slicer.org/Slicer3-lib-mirrors/trunk/weave
  BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/weave
  SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/weave
  BUILD_COMMAND ${CMAKE_COMMAND}
    -P ${CMAKE_CURRENT_BINARY_DIR}/weave_make_step.cmake
  CONFIGURE_COMMAND ""
  UPDATE_COMMAND ""
  INSTALL_COMMAND ""
  DEPENDS
    ${weave_DEPENDENCIES}
  )

