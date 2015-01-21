
#
# Adapted from CMake/Modules/CTest.cmake
#

macro(SlicerMacroGetCompilerName COMPILER_NAME_VAR)
  if(${COMPILER_NAME_VAR} STREQUAL "")
    message(FATAL_ERROR "error: COMPILER_NAME_VAR CMake variable is empty !")
  endif()

  set(DART_COMPILER "${CMAKE_CXX_COMPILER}")
  if(NOT DART_COMPILER)
    set(DART_COMPILER "${CMAKE_C_COMPILER}")
  endif()
  if(NOT DART_COMPILER)
    set(DART_COMPILER "unknown")
  endif()
  if(WIN32)
    set(DART_NAME_COMPONENT "NAME_WE")
  else()
    set(DART_NAME_COMPONENT "NAME")
  endif()
  if(NOT BUILD_NAME_SYSTEM_NAME)
    set(BUILD_NAME_SYSTEM_NAME "${CMAKE_SYSTEM_NAME}")
  endif()
  if(WIN32)
    set(BUILD_NAME_SYSTEM_NAME "Win32")
  endif()
  if(UNIX OR BORLAND)
    get_filename_component(DART_CXX_NAME
      "${CMAKE_CXX_COMPILER}" ${DART_NAME_COMPONENT})
  else()
    get_filename_component(DART_CXX_NAME
      "${CMAKE_BUILD_TOOL}" ${DART_NAME_COMPONENT})
  endif()
  if(DART_CXX_NAME MATCHES "msdev")
    set(DART_CXX_NAME "vs60")
  endif()
  if(DART_CXX_NAME MATCHES "devenv")
    # Adapted from CTest.cmake - Function GET_VS_VERSION_STRING
    string(REGEX REPLACE "Visual Studio ([0-9][0-9]?)($|.*)" "\\1" NUMBER "${CMAKE_GENERATOR}")
    if("${CMAKE_GENERATOR}" MATCHES "Visual Studio 7 .NET 2003")
      set(DART_CXX_NAME "vs71")
    else()
      set(DART_CXX_NAME "vs${NUMBER}")
    endif()
  endif()
  string(REPLACE "c++" "g++" DART_CXX_NAME ${DART_CXX_NAME})
  set(${COMPILER_NAME_VAR} ${DART_CXX_NAME})
endmacro()

