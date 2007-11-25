PROJECT(TCLAP)

CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

INCLUDE (GenerateTCLAPConfig.cmake)

CONFIGURE_FILE(${TCLAP_SOURCE_DIR}/UseTCLAP.cmake.in
               ${TCLAP_BINARY_DIR}/UseTCLAP.cmake COPYONLY IMMEDIATE)


FILE(GLOB __files1 "${TCLAP_SOURCE_DIR}/include/tclap/*.h")
INSTALL(FILES ${__files1} DESTINATION include/tclap COMPONENT Development)
# Note "COPYING" is a filename containing the license file
INSTALL(FILES COPYING DESTINATION include/tclap COMPONENT Development)

INSTALL(FILES 
        ${TCLAP_BINARY_DIR}/UseTCLAP.cmake 
        ${TCLAP_BINARY_DIR}/install/TCLAPConfig.cmake 
        DESTINATION lib/tclap COMPONENT Development)
