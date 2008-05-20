set(launcher_machine ${CMAKE_SYSTEM_PROCESSOR}) 

if(${launcher_machine} MATCHES "i386")
  set(launcher_machine "x86") 
endif(${launcher_machine} MATCHES "i386")


if(NOT DEFINED launcher_platform)
  set(launcher_platform  ${CMAKE_SYSTEM_NAME}-${launcher_machine})
endif(NOT DEFINED launcher_platform)
if(${launcher_platform} MATCHES Windows)
  if(CMAKE_CL_64)
    set(launcher_platform  win64)
  else(CMAKE_CL_64)
    set(launcher_platform  win32)
  endif(CMAKE_CL_64)
endif(${launcher_platform} MATCHES Windows)

if(${launcher_platform} MATCHES Linux)
  exec_program(uname ARGS -m OUTPUT_VARIABLE machine) 
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

#MESSAGE("launcher_platform is ${launcher_platform}\nlauncher name is ${launcher_name}")

set(launcher_binary
  "${Slicer3_BINARY_DIR}/Slicer3${CMAKE_EXECUTABLE_SUFFIX}")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/Slicer3Launchers/${launcher_name}"
  "${launcher_binary}"
  COPYONLY)

install(PROGRAMS "${launcher_binary}" DESTINATION ".")

# Should this location change, change Application/GUI/Slicer3.cxx as well

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/3DSlicerLogoICOimages/slicer3.ico"
  "${Slicer3_BINARY_DIR}/${Slicer3_INSTALL_SHARE_DIR}/slicer3.ico"
  COPYONLY)
install(
  FILES "${Slicer3_BINARY_DIR}/${Slicer3_INSTALL_SHARE_DIR}/slicer3.ico"
  DESTINATION ${Slicer3_INSTALL_SHARE_DIR}
  )


