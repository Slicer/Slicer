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

  if(NOT Slicer_CMake_HTTPS_Supported)
    file(DOWNLOAD ${url} ${dest} STATUS status)
    list(GET status 0 error_code)
    file(REMOVE ${dest})
    if(error_code)
      message(FATAL_ERROR "error: This CMake does not support the HTTPS protocol. Ensure that CMake is compiled with CMAKE_USE_OPENSSL enabled.")
    else()
      # Save result of check, this way there will be no need for a working network connection when reconfiguring
      set(Slicer_CMake_HTTPS_Supported TRUE CACHE INTERNAL "CMake supports HTTPS protocol")
    endif()
  endif()

  message(STATUS "${msg} - ok")
endfunction()

slicer_check_cmake_https()
