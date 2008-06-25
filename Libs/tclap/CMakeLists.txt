project(TCLAP)

cmake_minimum_required(VERSION 2.4)

if(NOT DEFINED BUILD_SHARED_LIBS)
  option(BUILD_SHARED_LIBS "Build with shared libraries." ON)
endif(NOT DEFINED BUILD_SHARED_LIBS)

include(GenerateTCLAPConfig.cmake)

configure_file(${TCLAP_SOURCE_DIR}/UseTCLAP.cmake.in
  ${TCLAP_BINARY_DIR}/UseTCLAP.cmake COPYONLY IMMEDIATE)


file(GLOB __files1 "${TCLAP_SOURCE_DIR}/include/tclap/*.h")
# Note "COPYING" is a filename containing the license file
install(FILES 
  ${__files1} 
  COPYING
  DESTINATION include/tclap COMPONENT Development
  )

install(FILES 
  ${TCLAP_BINARY_DIR}/UseTCLAP.cmake 
  ${TCLAP_BINARY_DIR}/install/TCLAPConfig.cmake 
  DESTINATION lib/tclap COMPONENT Development
  )
