
SET (launcher_machine ${CMAKE_SYSTEM_PROCESSOR}) 

IF (${launcher_machine} MATCHES "i386")
  SET (launcher_machine "x86") 
ENDIF (${launcher_machine} MATCHES "i386")


IF(NOT DEFINED launcher_platform )
  SET(launcher_platform  ${CMAKE_SYSTEM_NAME}-${launcher_machine})
ENDIF(NOT DEFINED launcher_platform )
IF(${launcher_platform} MATCHES Windows)
  IF(CMAKE_CL_64)
    SET(launcher_platform  win64)
  ELSE(CMAKE_CL_64)
    SET(launcher_platform  win32)
  ENDIF(CMAKE_CL_64)
ENDIF(${launcher_platform} MATCHES Windows)

IF(${launcher_platform} MATCHES Linux)
  EXEC_PROGRAM( uname ARGS -m OUTPUT_VARIABLE machine ) 
  IF (${machine} MATCHES "i686")
    SET (machine "x86") 
  ENDIF (${machine} MATCHES "i686")
  SET(launcher_platform  linux-${machine})
ENDIF(${launcher_platform} MATCHES Linux)

IF(${launcher_platform} MATCHES Darwin-powerpc)
  SET(launcher_platform darwin-ppc)
ENDIF(${launcher_platform} MATCHES Darwin-powerpc)


STRING ( TOLOWER "${launcher_platform}" launcher_platform )
SET (launcher_name "Slicer3-${launcher_platform}${CMAKE_EXECUTABLE_SUFFIX}")

#MESSAGE ( "launcher_platform is ${launcher_platform}\nlauncher name is ${launcher_name}" )

SET ( launcher_binary
  "${Slicer3_BINARY_DIR}/Slicer3${CMAKE_EXECUTABLE_SUFFIX}" )

CONFIGURE_FILE ( "${CMAKE_CURRENT_SOURCE_DIR}/Slicer3Launchers/${launcher_name}"
                 "${launcher_binary}"
                 COPYONLY )

INSTALL ( PROGRAMS "${launcher_binary}" DESTINATION ${Slicer3_BINARY_DIR} )

CONFIGURE_FILE ( "${CMAKE_CURRENT_SOURCE_DIR}/3DSlicerLogoICOimages/slicer3.ico"
                 "${Slicer3_BINARY_DIR}/lib"
                 COPYONLY )
INSTALL(
  FILES ${Slicer3_BINARY_DIR}/lib/slicer3.ico
  DESTINATION lib
)


