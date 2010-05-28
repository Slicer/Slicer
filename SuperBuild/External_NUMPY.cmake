# The Numpy external project 

set(numpy_source "${CMAKE_CURRENT_BINARY_DIR}/NUMPY")
set(numpy_binary "${CMAKE_CURRENT_BINARY_DIR}/NUMPY")

# to configure numpy we run a cmake -P script
# the script will create a site.cfg file
# then run python setup.py config to verify setup
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/NUMPY_configure_step.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/NUMPY_configure_step.cmake @ONLY)
# to build numpy we also run a cmake -P script.
# the script will set LD_LIBRARY_PATH so that 
# python can run after it is built on linux
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/NUMPY_make_step.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/NUMPY_make_step.cmake @ONLY)

# create an external project to download numpy,
# and configure and build it
ExternalProject_Add(NUMPY
  DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}
  SOURCE_DIR ${numpy_source}
  BINARY_DIR ${numpy_binary}
  CONFIGURE_COMMAND ${CMAKE_COMMAND}
    -P ${CMAKE_CURRENT_BINARY_DIR}/NUMPY_configure_step.cmake
  BUILD_COMMAND ${CMAKE_COMMAND}
    -P ${CMAKE_CURRENT_BINARY_DIR}/NUMPY_make_step.cmake
  SVN_REPOSITORY http://svn.scipy.org/svn/numpy/trunk
  INSTALL_COMMAND ""
  DEPENDS ${numpy_DEPENDENCIES}
  )
