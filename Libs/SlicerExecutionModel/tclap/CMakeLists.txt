project(TCLAP)

#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.2)
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# See http://cmake.org/cmake/help/cmake-2-8-docs.html#section_Policies for details
#-----------------------------------------------------------------------------

SET(project_policies
  CMP0001 # NEW: CMAKE_BACKWARDS_COMPATIBILITY should no longer be used.
  CMP0002 # NEW: Logical target names must be globally unique.
  CMP0003 # NEW: Libraries linked via full path no longer produce linker search paths.
  CMP0004 # NEW: Libraries linked may NOT have leading or trailing whitespace.
  CMP0005 # NEW: Preprocessor definition values are now escaped automatically.
  CMP0006 # NEW: Installing MACOSX_BUNDLE targets requires a BUNDLE DESTINATION.
  CMP0007 # NEW: List command no longer ignores empty elements.
  CMP0008 # NEW: Libraries linked by full-path must have a valid library file name.
  CMP0009 # NEW: FILE GLOB_RECURSE calls should not follow symlinks by default.
  CMP0010 # NEW: Bad variable reference syntax is an error.
  CMP0011 # NEW: Included scripts do automatic cmake_policy PUSH and POP.
  CMP0012 # NEW: if() recognizes numbers and boolean constants.
  CMP0013 # NEW: Duplicate binary directories are not allowed.
  CMP0014 # NEW: Input directories must have CMakeLists.txt
  )
FOREACH(policy ${project_policies})
  IF(POLICY ${policy})
    CMAKE_POLICY(SET ${policy} NEW)
  ENDIF()
ENDFOREACH()


## No libs created in tclap if(NOT DEFINED BUILD_SHARED_LIBS)
## No libs created in tclap   option(BUILD_SHARED_LIBS "Build with shared libraries." ON)
## No libs created in tclap endif(NOT DEFINED BUILD_SHARED_LIBS)

include(GenerateTCLAPConfig.cmake)

configure_file(${TCLAP_SOURCE_DIR}/UseTCLAP.cmake.in
  ${TCLAP_BINARY_DIR}/UseTCLAP.cmake COPYONLY)


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
