#
# slicer_check_cmake_https
#
# Check if CMake supports downloading files using the HTTPS protocol. Fail if
# HTTPS is unsupported.
#
# CMake should support HTTPS when compiled with CMAKE_USE_OPENSSL enabled.
#
# Based on the script created by Jean-Christophe Fillion-Robin:
# https://gist.github.com/jcfr/e88a2a7cbc4ddd235186
#

function(slicer_check_cmake_https)
  set(url "https://raw.githubusercontent.com/Slicer/Slicer/master/CMakeLists.txt")
  set(dest "${CMAKE_CURRENT_BINARY_DIR}/slicer_check_cmake_https_output")

  set(msg "Checking if CMake supports https")
  message(STATUS "${msg}")
  file(DOWNLOAD ${url} ${dest} STATUS status)
  list(GET status 0 error_code)
  file(REMOVE ${dest})
  if(error_code)
    message(FATAL_ERROR "error: This CMake does not support the HTTPS protocol. Ensure that CMake is compiled with CMAKE_USE_OPENSSL enabled.")
  endif()
  message(STATUS "${msg} - ok")
endfunction()
