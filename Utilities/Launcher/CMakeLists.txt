set(launcher_machine ${CMAKE_SYSTEM_PROCESSOR}) 

if(${launcher_machine} MATCHES "i386")
  set(launcher_machine "x86") 
endif(${launcher_machine} MATCHES "i386")


if(NOT DEFINED launcher_platform)
  set(launcher_platform  ${CMAKE_SYSTEM_NAME}-${launcher_machine})
endif(NOT DEFINED launcher_platform)
if(${launcher_platform} MATCHES Windows)
  if(CMAKE_CL_64)
    # set(launcher_platform  win64)
    # since Slicer3-win64.exe has not been created yet (Steve?), let's use
    # the win32 one.
    set(launcher_platform  win32)
  else(CMAKE_CL_64)
    set(launcher_platform  win32)
  endif(CMAKE_CL_64)
endif(${launcher_platform} MATCHES Windows)

if(${launcher_platform} MATCHES Linux)
  execute_process(COMMAND uname -m OUTPUT_VARIABLE machine OUTPUT_STRIP_TRAILING_WHITESPACE)
  if(${machine} MATCHES "i686")
    set(machine "x86") 
  endif(${machine} MATCHES "i686")
  set(launcher_platform  linux-${machine})
endif(${launcher_platform} MATCHES Linux)

if(${launcher_platform} MATCHES Darwin-powerpc)
  set(launcher_platform darwin-ppc)
endif(${launcher_platform} MATCHES Darwin-powerpc)


string(TOLOWER "${launcher_platform}" launcher_platform)
set(launcher_name "Slicer3-${launcher_platform}${CMAKE_EXECUTABLE_SUFFIX}")

set(launcher_source
  "${CMAKE_CURRENT_SOURCE_DIR}/Slicer3Launchers/${launcher_name}")

SET(launcher_binary
  "${Slicer_BINARY_DIR}/Slicer4${CMAKE_EXECUTABLE_SUFFIX}")

# MESSAGE(STATUS "launcher_platform: ${launcher_platform}")
# MESSAGE(STATUS "launcher_name: ${launcher_name}")
# MESSAGE(STATUS "launcher_source: ${launcher_source}")
# MESSAGE(STATUS "launcher_binary: ${launcher_binary}")

if(EXISTS "${launcher_source}")
  configure_file("${launcher_source}" "${launcher_binary}" COPYONLY)
  install(PROGRAMS "${launcher_binary}" DESTINATION ".")
endif(EXISTS "${launcher_source}")

# Should this location change, change Application/GUI/Slicer3.cxx as well

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/3DSlicerLogoICOimages/slicer3.ico"
  "${Slicer_BINARY_DIR}/${Slicer_INSTALL_SHARE_DIR}/Resources/slicer3.ico"
  COPYONLY)
install(
  FILES "${CMAKE_CURRENT_SOURCE_DIR}/3DSlicerLogoICOimages/slicer3.ico"
  DESTINATION ${Slicer_INSTALL_SHARE_DIR}/Resources
  )
