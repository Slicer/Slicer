PROJECT(TCLAP)

CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

CONFIGURE_FILE(
  ${TCLAP_SOURCE_DIR}/TCLAPConfig.cmake.in 
  ${TCLAP_BINARY_DIR}/TCLAPConfig.cmake
)

FILE(GLOB __files1 "${TCLAP_SOURCE_DIR}/include/tclap/*.h")
INSTALL(FILES ${__files1} DESTINATION include/tclap COMPONENT Development)
# Note "COPYING" is a filename containing the license file
INSTALL(FILES COPYING DESTINATION include/tclap COMPONENT Development)
