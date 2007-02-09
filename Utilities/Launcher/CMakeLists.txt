
IF(NOT DEFINED launcher_platform )
  SET(launcher_platform  ${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR})
ENDIF(NOT DEFINED launcher_platform )
IF(${launcher_platform} MATCHES Windows)
  IF(CMAKE_CL_64)
    SET(launcher_platform  win64)
  ELSE(CMAKE_CL_64)
    SET(launcher_platform  win32)
  ENDIF(CMAKE_CL_64)
ENDIF(${launcher_platform} MATCHES Windows)

STRING ( TOLOWER "${launcher_platform}" launcher_platform )
SET (launcher_name "Slicer3-${launcher_platform}")

MESSAGE ( "launcher name is ${launcher_name}" )

SET ( launcher_binary
  "${Slicer3_BINARY_DIR}/Slicer3${CMAKE_EXECUTABLE_SUFFIX}" )

CONFIGURE_FILE ( "${CMAKE_CURRENT_SOURCE_DIR}/${launcher_name}"
                 "${launcher_binary}"
                 COPYONLY )

INSTALL ( PROGRAMS "${launcher_binary}" DESTINATION "" )

