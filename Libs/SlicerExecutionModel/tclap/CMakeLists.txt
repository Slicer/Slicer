project(TCLAP)

#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.2)
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# See http://cmake.org/cmake/help/cmake-2-8-docs.html#section_Policies for details
#-----------------------------------------------------------------------------
if(POLICY CMP0016)
  cmake_policy(SET CMP0016 NEW)
endif()
if(POLICY CMP0017)
  cmake_policy(SET CMP0017 OLD)
endif()

# --------------------------------------------------------------------------
# Configure support file
# --------------------------------------------------------------------------
include(GenerateTCLAPConfig.cmake)

configure_file(
  ${TCLAP_SOURCE_DIR}/UseTCLAP.cmake.in
  ${TCLAP_BINARY_DIR}/UseTCLAP.cmake COPYONLY
  )

# --------------------------------------------------------------------------
# Install headers
# --------------------------------------------------------------------------
if(NOT DEFINED ${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT)
  set(${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT ON)
endif()
if(NOT ${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT)
  file(GLOB headers "${TCLAP_SOURCE_DIR}/include/tclap/*.h")
  # Note "COPYING" is a filename containing the license file
  install(
    FILES ${headers} COPYING
    DESTINATION include/tclap COMPONENT Development
    )
endif()

# --------------------------------------------------------------------------
# Install support files
# --------------------------------------------------------------------------
if(NOT ${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT)
  install(FILES
    ${TCLAP_BINARY_DIR}/UseTCLAP.cmake
    ${TCLAP_BINARY_DIR}/install/TCLAPConfig.cmake
    DESTINATION lib/tclap COMPONENT Development
    )
endif()

# --------------------------------------------------------------------------
# Set INCLUDE_DIRS variable
# --------------------------------------------------------------------------
set(${PROJECT_NAME}_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
  CACHE INTERNAL "${PROJECT_NAME} include dirs" FORCE)
