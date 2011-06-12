
# Set dependency list
set(weave_DEPENDENCIES python NUMPY)

set(proj weave)
include(${Slicer_SOURCE_DIR}/CMake/SlicerBlockCheckExternalProjectDependencyList.cmake)
set(${proj}_EXTERNAL_PROJECT_INCLUDED TRUE)

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
